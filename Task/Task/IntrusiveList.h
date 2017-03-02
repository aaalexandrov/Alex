#pragma once

template <class T>
class IntrusiveList {
public:
  typedef T Type;
  
  class iterator {
  public:
    iterator(T *e) : elem(e) {}
    T &operator *() { return *elem; }
    iterator &operator ++() { elem = elem->next(); return *this; }
    iterator &operator --() { elem = elem->prev(); return *this; }
    bool operator ==(const iterator& other) const { return elem == other.elem; }
    bool operator !=(const iterator&other) const { return !(*this == other); }
  private:
    T *elem;
  };

  iterator begin() { return iterator(m_First); }
  iterator end()   { return iterator(nullptr); }

  void insert(T &elem, T *prev = nullptr)
  {
    T *&prevNext = prev != nullptr ? prev->next() : m_First;
    T *&nextPrev = prevNext != nullptr ? prevNext->prev() : m_Last;
    elem.next() = prevNext;
    elem.prev() = prev;
    nextPrev = prevNext = &elem;
    ++m_Count;
  }

  void remove(T &elem)
  {
    T *&prevNext = elem.prev() != nullptr ? elem.prev()->next() : m_First;
    T *&nextPrev = elem.next() != nullptr ? elem.next()->prev() : m_Last;
    prevNext = elem.next();
    nextPrev = elem.prev();
    --m_Count;
  }

  void insert_before(T &elem, T *next = nullptr)
  {
    insert(elem, next != nullptr ? next->prev() : m_Last);
  }

  void clear()
  {
    m_First = m_Last = nullptr;
  }

  T *first() const { return m_First; }
  T *last() const  { return m_Last;  }

  size_t count() const { return m_Count; }

private:
  T      *m_First = nullptr, *m_Last = nullptr;
  size_t  m_Count = 0;
};