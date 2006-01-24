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
	 m_osem (new yans::Semaphore (0))
  {}
  ~B () {delete m_sem; delete m_osem;}

  void wait_until_notify (void) {
    m_sem->down ();
    m_osem->up ();
  }
private:
  virtual void run (void) {
    TRACE ("B run");
    double test_float = 100.0;
    sleep_s (10.0);
    m_sem->up ();
    m_osem->down ();
    if (test_float != 100.0) {
      TRACE ("Float problem !!");
    }
    TRACE ("B run completed");
  }
  yans::Semaphore *m_sem;
  yans::Semaphore *m_osem;
};

class C : public yans::Thread {
public:
  C (B *b) : yans::Thread ("C"), m_b (b) {}
private:
  virtual void run (void) {
    TRACE ("C run");
    double test_float = 200.0;
    m_b->wait_until_notify ();
    if (test_float != 200.0) {
      TRACE ("Float problem !!");
    }
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
