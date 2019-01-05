#include <am.h>
#include <x86.h>
#include <klib.h>

static _Context* (*user_handler)(_Event, _Context*) = NULL;
//for pa4.2
extern void get_cur_as(_Context *c);
extern void _switch(_Context *c);

void vectrap();
void vecnull();
void vecsys();

_Context* irq_handle(_Context *tf) {
  _Context *next = tf;
   // printf("cpu eax:0x%08x\n", tf->eax);
   // printf("cpu ebx:0x%08x\n", tf->ebx);
   // printf("cpu ecx:0x%08x\n", tf->ecx);
   // printf("cpu edx:0x%08x\n", tf->edx);
  get_cur_as(tf);
  if (user_handler) {
    _Event ev = {0};

    switch (tf->irq) {
      case 0x80: ev.event = _EVENT_SYSCALL; break;
      case 0x81: ev.event = _EVENT_YIELD; break;
      default: ev.event = _EVENT_ERROR; break;
    }

    next = user_handler(ev, tf);
    if (next == NULL) {
      next = tf;
    }
  }
  _switch(next);
  return next;
}

static GateDesc idt[NR_IRQ];

int _cte_init(_Context*(*handler)(_Event, _Context*)) {
  // initialize IDT
  for (unsigned int i = 0; i < NR_IRQ; i ++) {
    idt[i] = GATE(STS_TG32, KSEL(SEG_KCODE), vecnull, DPL_KERN);
  }

  // -------------------- system call --------------------------
  idt[0x80] = GATE(STS_TG32, KSEL(SEG_KCODE), vecsys, DPL_KERN);
  idt[0x81] = GATE(STS_TG32, KSEL(SEG_KCODE), vectrap, DPL_KERN);

  set_idt(idt, sizeof(idt));

  // register event handler
  user_handler = handler;

  return 0;
}

_Context *_kcontext(_Area stack, void (*entry)(void *), void *arg) {
  _Context* ct = (stack.end - sizeof(_Context));
  memset(ct, 0, sizeof(_Context));
  ct->eip = (uintptr_t)entry;
  ct->cs = 8;
  printf("_kcontext\n");
  return ct;
}

void _yield() {
  //printf("here\n");
  asm volatile("int $0x81");
}

int _intr_read() {
  return 0;
}

void _intr_write(int enable) {
}
