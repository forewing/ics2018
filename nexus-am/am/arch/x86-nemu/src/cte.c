#include <am.h>
#include <x86.h>
#include <klib.h>
static _Context* (*user_handler)(_Event, _Context*) = NULL;

void vectrap();
void vecsys();
void vecnull();

_Context* irq_handle(_Context *tf) {
  _Context *next = tf;
  if (user_handler) {
    _Event ev = {0};
    // printf("edi:%d\n", tf->edi);
    // printf("esi:%d\n", tf->esi);
    // printf("ebp:%d\n", tf->ebp);
    // printf("esp:%d\n", tf->esp);
    // printf("ebx:%d\n", tf->ebx);
    // printf("edx:%d\n", tf->edx);
    // printf("ecx:%d\n", tf->ecx);
    // printf("eax:%d\n", tf->eax);
    // printf("irq:%d\n", tf->irq);
    // printf("err:%d\n", tf->err);
    // printf("eip:%d\n", tf->eip);
    // printf("cs:%d\n", tf->cs);
    // printf("eflags:%d\n", tf->eflags);
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

  return next;
}

static GateDesc idt[NR_IRQ];

int _cte_init(_Context*(*handler)(_Event, _Context*)) {
  // initialize IDT
  for (unsigned int i = 0; i < NR_IRQ; i ++) {
    idt[i] = GATE(STS_TG32, KSEL(SEG_KCODE), vecnull, DPL_KERN);
  }

  // -------------------- system call --------------------------
  idt[0x81] = GATE(STS_TG32, KSEL(SEG_KCODE), vectrap, DPL_KERN);
  idt[0x80] = GATE(STS_TG32, KSEL(SEG_KCODE), vecsys, DPL_KERN);

  set_idt(idt, sizeof(idt));

  // register event handler
  user_handler = handler;

  return 0;
}

_Context *_kcontext(_Area stack, void (*entry)(void *), void *arg) {
  return NULL;
}

void _yield() {
  asm volatile("int $0x81");
}

int _intr_read() {
  return 0;
}

void _intr_write(int enable) {
}
