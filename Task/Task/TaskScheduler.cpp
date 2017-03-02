#include "stdafx.h"

#include <assert.h>
#include "TaskScheduler.h"

Task::Task(Task *parent, void *user, std::function<void(Task&)> taskFn, size_t stackSize, TaskScheduler *scheduler, bool deleteOnFinish) :
  m_Scheduler(scheduler), m_Child(nullptr), m_Func(taskFn), m_State(State::Runnable), m_DeleteOnFinish(deleteOnFinish)
{
  assert((void *) this == &m_Context);
  void *stack = stackSize > 0 ? malloc(stackSize) : nullptr;
  task_init(&m_Context, &parent->m_Context, user, (taskfn) TaskScheduler::TaskFunc, stack, stackSize);
}

Task::~Task()
{
  free(m_Context.stack_base);
}

void Task::Yield(Task *next)
{
}

TaskScheduler::TaskScheduler()
{
  m_Main = CreateTask(nullptr, nullptr, nullptr, true, false, 0);
  m_Current = m_Main;
}

TaskScheduler::~TaskScheduler()
{
  assert(m_Current == m_Main);
  for (Task *task : m_Tasks)
    delete task;
}

Task *TaskScheduler::CreateTask(std::function<void(Task&)> taskFn, void *user, Task *parent, bool schedule, bool deleteOnFinish, size_t stackSize)
{
  Task *task = new Task(parent, user, taskFn, stackSize, this, deleteOnFinish);
  m_Tasks.insert(task);
  if (schedule)
    SetState(*task, Task::State::Scheduled);
  return task;
}

void TaskScheduler::DeleteTask(Task *task)
{
  assert(task != m_Current);
  if (task->m_State == Task::State::Scheduled)
    m_Scheduled.remove(*task);
  m_Tasks.erase(task);
  delete task;
}

void TaskScheduler::YieldTo(Task &task)
{
  assert(task.m_State == Task::State::Runnable || task.m_State == Task::State::Scheduled);
  Task *current = m_Current;
  task_swap(&m_Current->m_Context, &task.m_Context);
  m_Current = current;
}

void TaskScheduler::SetState(Task &task, Task::State state)
{
  if (task.m_State != state) {
    if (task.m_State == Task::State::Scheduled)
      m_Scheduled.remove(task);
    else
      if (state == Task::State::Scheduled) {
        assert(task.m_State == Task::State::Runnable);
        m_Scheduled.insert_before(task, m_Scheduled.last()); // insert the task just before the main task, which should be the last
      }
  }
}

void TaskScheduler::RunScheduled()
{
  assert(m_Current == m_Main);
  assert(m_Scheduled.last() == m_Main);
  YieldTo(*m_Scheduled.first());
}


void TaskScheduler::TaskFunc(Task *task)
{
  assert(task->m_State == Task::State::Scheduled || task->m_State == Task::State::Runnable);
  TaskScheduler *scheduler = task->m_Scheduler;
  scheduler->m_Current = task;
  task->m_Func(*task);
  scheduler->SetState(*task, Task::State::Finished);
}
