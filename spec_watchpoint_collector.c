#include <linux/module.h>
#include <linux/smp.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/uaccess.h>
#include <linux/perf_event.h>
#include <linux/slab.h>

#include <asm/io.h>

#define EVENT_ID 0x01B0 // ARITH.FPDIV_ACTIVE

// List of CPUs to configure
const int cpu_list[] = {2, 3, 4};

static void event_handler(struct perf_event *evt,
       struct perf_sample_data *data, struct pt_regs *regs)
{
   printk(KERN_INFO "[@] Watchpoint signal @ 0x%08lx\n", regs->ip);
}

static DEFINE_PER_CPU(struct perf_event *, perf_event);

static int __init kmod_init(void)
{
  int cpu = 0;
  int cpu_id = 0;
  struct perf_event *evt;
  static struct perf_event_attr event_attr;

  for (; cpu_id < (sizeof(cpu_list) / sizeof(int)); cpu_id++) {
    cpu = cpu_list[cpu_id];

    // Prepare perf event struct
		memset(&event_attr, 0, sizeof(struct perf_event_attr));
		event_attr.type = PERF_TYPE_RAW;
		event_attr.size = sizeof(struct perf_event_attr);
		event_attr.config = EVENT_ID;
		event_attr.sample_period = 1;
		event_attr.exclude_kernel = 1;

    // Configure PMU
    printk(KERN_INFO "spec_watchpoint_collector: Configure PMU for CPU %d\n", cpu);
    evt = perf_event_create_kernel_counter(&event_attr, cpu, NULL, &event_handler, NULL);
    if (evt == NULL) {
      printk(KERN_INFO "spec_watchpoint_collector: Failed to configure PMU for CPU %d\n", cpu);
      break;
    }

    per_cpu(perf_event, cpu) = evt;
  }

  return 0;
}

static void __exit kmod_exit(void)
{
  int cpu = 0;
  int cpu_id = 0;

  // Cleanup
  for (; cpu_id < (sizeof(cpu_list) / sizeof(int)); cpu_id++) {
    cpu = cpu_list[cpu_id];

    printk(KERN_INFO "spec_watchpoint_collector: Release PMU for CPU %d\n", cpu);
    perf_event_release_kernel(per_cpu(perf_event, cpu));
  }
}

module_init(kmod_init);
module_exit(kmod_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Maximilian Barger");
MODULE_DESCRIPTION("This kernel module collects signals from speculative watchpoints");
