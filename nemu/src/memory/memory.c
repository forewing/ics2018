#include "nemu.h"
#include "device/mmio.h"

#define PMEM_SIZE (128 * 1024 * 1024)

#define pmem_rw(addr, type) *(type *)({\
    Assert(addr < PMEM_SIZE, "physical address(0x%08x) is out of bound, eip=%x, total=%llu", addr, cpu.eip, (unsigned long long)get_steps()); \
    guest_to_host(addr); \
    })

uint8_t pmem[PMEM_SIZE];

/* Memory accessing interfaces */

uint32_t paddr_read(paddr_t addr, int len) {
  // if (addr >= 0x7a3c && addr < 0x7a40){
  //   printf(" read  addr: %x, len: %x eip: %x data: %x", addr, len, cpu.eip, pmem_rw(addr, uint32_t) & (~0u >> ((4 - len) << 3)));
  //   monitor_statistic();
  // }
  int mmio_map = is_mmio(addr);
  if (mmio_map == -1){
    return pmem_rw(addr, uint32_t) & (~0u >> ((4 - len) << 3));
  }else{
    return mmio_read(addr, len, mmio_map);
  }
}

void paddr_write(paddr_t addr, uint32_t data, int len) {
  // target: 0x7a3c
  // if (addr >= 0x7a3c && addr < 0x7a40){
  //   printf("write addr: %x, data: %x, len: %x eip: %x", addr, data, len, cpu.eip);
  //   monitor_statistic();
  // }
  int mmio_map = is_mmio(addr);
  if (mmio_map == -1){
    memcpy(guest_to_host(addr), &data, len);
  }else{
    mmio_write(addr, len, data, mmio_map);
  }
}

uint32_t vaddr_read(vaddr_t addr, int len) {
  return paddr_read(addr, len);
}

void vaddr_write(vaddr_t addr, uint32_t data, int len) {
  paddr_write(addr, data, len);
}
