#include "thread.h"
#include "semaphore.h"
#include "simulator.h"

#include <iostream>
# define TRACE(x) \
  std::cout << "TEST THREAD " << yans::Simulator::now_s () << " " << x << std::endl;


class A : public yans::Thread {
public:
  A () : yans::Thread ("A") {}
private:
  virtual void run (void) {
    TRACE ("A run");
    sleep_s (1.0);
    TRACE ("A run completed");
  }
};

class B : public yans::Thread {
public:
  B () : yans::Thread ("B"), 
	 m_sem (new yans::Semaphore (0)),
	 m_osem (new yans::Semaphore (0)),
	 m_oosem (new yans::Semaphore (0))
  {}
  ~B () {delete m_sem; delete m_osem; delete m_oosem;}

  void wait_until_notify (void) {
    m_sem->down ();
    register double test_float = 200.0;
    m_osem->up ();
    m_oosem->down ();
    if (test_float != 200.0) {
      TRACE ("Float problem !!");
    }
  }
private:
  virtual void run (void) {
    TRACE ("B run");
    register double test_float = 100.0;
    sleep_s (10.0);
    m_sem->up ();
    m_osem->down ();
    if (test_float != 100.0) {
      TRACE ("Float problem !!");
    }
    m_oosem->up ();
    TRACE ("B run completed");
  }
  yans::Semaphore *m_sem;
  yans::Semaphore *m_osem;
  yans::Semaphore *m_oosem;
};

class C : public yans::Thread {
public:
  C (B *b) : yans::Thread ("C"), m_b (b) {}
private:
  virtual void run (void) {
    TRACE ("C run");
    m_b->wait_until_notify ();
    TRACE ("C completed");
  }
  B* m_b;
};


int main (int argc, char *argv[])
{
  yans::Thread *a = new A ();

  TRACE ("a created");

  yans::Simulator::run ();

  TRACE ("simulation completed");

  delete a;

  yans::Simulator::destroy ();


  B *b = new B ();
  C *c = new C (b);

  yans::Simulator::run ();

  yans::Simulator::destroy ();

  delete b;
  delete c;

  return 0;
}
