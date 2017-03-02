#pragma once

#include "TaskContext.h"
#include <functional>
#include <set>
#include "IntrusiveList.h"

class TaskScheduler;
class Task {
public:
  enum class State {
    Runnable,
    Scheduled,
    Waiting,
    Finished,

    Count,
  };

  void Yield(Task *next = nullptr);

  State GetState() const { return m_State; }

  TaskScheduler *GetScheduler() const { return m_Scheduler; }

  Task *&next() { return *(Task **) &m_Context.parent; }
  Task *&prev() { return m_Child; }

protected:
  Task(Task *parent, void *user, std::function<void(Task&)> taskFn, size_t stackSize, TaskScheduler *scheduler, bool deleteOnFinish);
  ~Task();

  TaskContext                 m_Context;
  Task                       *m_Child;
  TaskScheduler              *m_Scheduler;
  std::function<void(Task&)>  m_Func;
  State                       m_State;
  bool                        m_DeleteOnFinish;

  friend class TaskScheduler;
};

class TaskScheduler {
public:
  static const size_t DefaultStackSize = 16384;

  TaskScheduler();
  ~TaskScheduler();

  Task *CreateTask(std::function<void(Task&)> taskFn, void *user = nullptr, Task *parent = nullptr, bool schedule = true, bool deleteOnFinish = true, size_t stackSize = DefaultStackSize);
  void DeleteTask(Task *task);

  Task &GetMain() const { return *m_Main; }
  Task &GetCurrent() const { return *m_Current; }

  void YieldTo(Task &task);
  void SetState(Task &task, Task::State state);

  void RunScheduled();

  static void TaskFunc(Task *task);
protected:

  IntrusiveList<Task>  m_Scheduled;
  std::set<Task*>      m_Tasks;
  Task                *m_Main, *m_Current;
};