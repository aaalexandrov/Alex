#pragma once

struct TaskContext {
  void        *user;
  TaskContext *parent;
  void        *stack_ptr;
  void        *stack_base;
  size_t       stack_size;
};

typedef void(*taskfn)(TaskContext *task);

void task_init(TaskContext *task, TaskContext *parent = 0, void *user = 0, taskfn fn = 0, void *stack_base = 0, size_t stack_size = 0);
inline bool task_finished(TaskContext *task) { return !task->stack_ptr; }

extern "C" {
  void task_associate(TaskContext *task, taskfn fn);
  void task_swap(TaskContext *from, TaskContext *to);
}
