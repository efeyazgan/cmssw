#ifndef Tracker_ProxyBase11_H
#define Tracker_ProxyBase11_H

#include "FWCore/Utilities/interface/Visibility.h"
#include "FWCore/Utilities/interface/Likely.h"
#include "FWCore/Utilities/interface/GCC11Compatibility.h"

#ifndef CMS_NOCXX11
#include "ChurnAllocator.h"
#endif

/** A base class for reference counting proxies.
 *  The class to which this one is proxy must inherit from
 *  ReferenceCounted.
 *  A ProxyBase has value semantics, in contrast to ReferenceCountingPointer,
 *  which has pointer semantics.
 *  The Proxy class inheriting from ProxyBase must duplicate the
 *  part of the interface of the reference counted class that it whiches to expose.
 */


template <class T> 
class ProxyBase11 {
public:
  
#ifdef CMS_NOCXX11
  typedef T * pointer;
#else
  using pointer = std::shared_ptr<T>;
#endif

  // protected:
  
  ProxyBase11() {}
  
  explicit ProxyBase11( T* p) : theData(p) {}
#ifndef CMS_NOCXX11
  template<typename U>
  ProxyBase11(std::shared_ptr<U> p) : theData(std::move(p)){}
  template<typename U>
  ProxyBase11& operator=(std::shared_ptr<U> p) { theData =std::move(p); return *this;}
#endif  

  ~ProxyBase11()  noexcept {
    destroy();
  }
  
  void swap(ProxyBase11& other)  noexcept {
    std::swap(theData,other.theData);
  }

#ifdef CMS_NOCXX11
  ProxyBase11& operator=( const ProxyBase11& other) {
    return *this;
  }
  ProxyBase11( const ProxyBase11& other) {}
#else
  ProxyBase11(ProxyBase11&& other)  noexcept = default;
  ProxyBase11& operator=(ProxyBase11&& other)  noexcept = default; 
  ProxyBase11(ProxyBase11 const & other) = default;
  ProxyBase11& operator=( const ProxyBase11& other) = default;
#endif

  void reset() { theData.reset();}

  const T& data() const { check(); return *theData;}

  T& unsharedData() {
    check(); 
    if ( references() > 1) {
      theData = theData->clone();
    }
    return *theData;
  }

  T& sharedData() { check(); return *theData;}

  bool isValid() const { return bool(theData);}

  void check() const {
#ifdef TR_DEBUG
    if  unlikely(!theData)
      throw TrajectoryStateException("Error: uninitialized ProxyBase11 used");
#endif
  }

  void destroy()  noexcept {}
#ifndef CMS_NOCXX11
  int  references() const {return theData.use_count();}
#else
  int  references() const { return 0;}
#endif

private:
#ifdef CMS_NOCXX11
  T *  theData;
#else
  std::shared_ptr<T> theData;
#endif
};

template <class T >
inline
void swap(ProxyBase11<T>& lh, ProxyBase11<T>& rh)  noexcept {
  lh.swap(rh);
}

#endif // Tracker_ProxyBase11_H
