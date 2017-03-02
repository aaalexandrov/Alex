#include "stdafx.h"

#include <malloc.h>
#include <assert.h>
#include "TaskContext.h"

void task_init(TaskContext *task, TaskContext *parent, void *user, taskfn fn, void *stack_base, size_t stack_size)
{
  assert(!fn || stack_size && stack_base);
  task->user = user;
  task->parent = parent;
  task->stack_base = stack_base;
  task->stack_size = stack_size;

  if (fn)
    task_associate(task, fn);
  else // this context can only be used to store the currently executing stack's execution context, stack_ptr != 0 means it's not finished
    task->stack_ptr = (void *)-1;
}

