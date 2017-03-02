#include "stdafx.h"
#include "TaskContext.h"
#include <iostream>

void task_alloc(TaskContext *task, TaskContext *parent = 0, void *user = 0, taskfn fn = 0, size_t stack_size = 0)
{
  task_init(task, parent, user, fn, stack_size > 0 ? malloc(stack_size) : 0, stack_size);
}

void task_free(TaskContext *task)
{
  free(task->stack_base);
}

void fib(TaskContext *task)
{
  int a = 0, b = 1;
  while (a < 100)
  {
    *(int*)(task->user) = a;
    task_swap(task, task->parent);
    int t = a;
    a = b;
    b = t + b;
  }
}

void task1(TaskContext *task)
{
  TaskContext fib_task;
  int param;
  std::cout << "Task 1" << std::endl;
  task_alloc(&fib_task, task, &param, fib, 16384);

  while (true)
  {
    task_swap(task, &fib_task);
    if (task_finished(&fib_task))
      break;
    std::cout << param << std::endl;
  }

  task_free(&fib_task);
  std::cout << "Task 1 signing off" << std::endl;
}

#include "IntrusiveList.h"

struct ListElem {
  int m_Data;
  ListElem *m_Next, *m_Prev;

  ListElem(int data) : m_Data(data), m_Next(nullptr), m_Prev(nullptr) {}
  ListElem *&next() { return m_Next; }
  ListElem *&prev() { return m_Prev; }
};

void testlist()
{
  ListElem el1(1), el2(2), el3(3);
  IntrusiveList<ListElem> list;
  list.insert(el1);
  list.insert(el3, list.last());
  list.insert(el2, list.first());

  for (auto e : list) {
    std::cout << e.m_Data << std::endl;
  }

  list.remove(el2);
  for (auto e : list) {
    std::cout << e.m_Data << std::endl;
  }
  list.remove(el1);
  for (auto e : list) {
    std::cout << e.m_Data << std::endl;
  }
  list.remove(*list.first());
  for (auto e : list) {
    std::cout << e.m_Data << std::endl;
  }
  std::cout << list.count() << std::endl;
}

int main()
{
  testlist();

  TaskContext main_task, other1;
  task_alloc(&main_task);
  task_alloc(&other1, &main_task, 0, task1, 16384);

  std::cout << "Starting" << std::endl;

  task_swap(&main_task, &other1);

  std::cout << "Ending" << std::endl;

  task_free(&other1);
  task_free(&main_task);

  std::cin.ignore();

  return 0;
}

