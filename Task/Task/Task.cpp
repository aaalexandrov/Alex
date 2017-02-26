#include "stdafx.h"
#include <iostream>


struct Task {
  void   *user;
  void   *stack_ptr;
  Task   *parent;
  void   *stack_base;
  size_t  stack_size;
};

typedef void(*taskfn)(Task *task);

extern "C" {
  void makecontext(Task *task, taskfn fn);
  void swapcontext(Task *from, Task *to);
}

void init_task(Task *task, Task *parent, void *stack_base, size_t stack_size, taskfn fn, void *user)
{
  task->user = user;
  task->stack_ptr = 0;
  task->parent = parent;
  task->stack_base = stack_base;
  task->stack_size = stack_size;

  // stack_base == 0 means currently executing stack, so no need to initialize anything there
  if (task->stack_base)
    makecontext(task, fn);
}

bool hasended_task(Task *task)
{
  return !task->parent && task->stack_base;
}

void create_task(Task *task, Task *parent, size_t stack_size, taskfn fn, void *user)
{
  init_task(task, parent, stack_size > 0 ? malloc(stack_size) : 0, stack_size, fn, user);
}

void delete_task(Task *task)
{
  free(task->stack_base);
}

void fib(Task *task)
{
  int a = 0, b = 1;
  while (a < 100)
  {
    *(int*)(task->user) = a;
    swapcontext(task, task->parent);
    int t = a;
    a = b;
    b = t + b;
  }
}

void task1(Task *task)
{
  Task fib_task;
  int param;
  std::cout << "Task 1" << std::endl;
  create_task(&fib_task, task, 16384, fib, &param);

  while (true)
  {
    swapcontext(task, &fib_task);
    if (hasended_task(&fib_task))
      break;
    std::cout << param << std::endl;
  }

  delete_task(&fib_task);
  std::cout << "Task 1 signing off" << std::endl;
}

int main()
{
  Task main_task, other1;
  create_task(&main_task, 0, 0, 0, 0);
  create_task(&other1, &main_task, 16384, task1, 0);

  std::cout << "Starting" << std::endl;

  swapcontext(&main_task, &other1);

  std::cout << "Ending" << std::endl;

  delete_task(&other1);
  delete_task(&main_task);

  std::cin.ignore();

  return 0;
}

