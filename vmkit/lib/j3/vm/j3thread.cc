#include "j3/j3thread.h"
#include "j3/j3method.h"
#include "j3/j3class.h"
#include "j3/j3.h"

#include "vmkit/safepoint.h"
#include "vmkit/compiler.h"

using namespace j3;

J3Thread::J3Thread(J3* vm, vmkit::BumpAllocator* allocator) : 
	Thread(vm, allocator),
	_localReferences(allocator) {
	_jniEnv.functions = &jniEnvTable;
}

J3Thread* J3Thread::create(J3* j3) {
	vmkit::BumpAllocator* allocator = vmkit::BumpAllocator::create();
	return new(allocator) J3Thread(j3, allocator);
}

void J3Thread::doRun() {
	J3ObjectHandle* handle = get()->javaThread();
	get()->vm()->threadRun->invokeVirtual(handle);
}

void J3Thread::start(J3ObjectHandle* handle) {
	J3Thread* thread = create(J3Thread::get()->vm());
	thread->assocJavaThread(handle);
	Thread::start(doRun, thread);
	while(1);
}

J3Method* J3Thread::getJavaCaller(uint32_t level) {
	vmkit::Safepoint* sf = 0;
	vmkit::StackWalker walker;

	while(walker.next()) {
		vmkit::Safepoint* sf = vm()->getSafepoint(walker.ip());

		if(sf && !level--)
			return ((J3MethodCode*)sf->unit()->getSymbol(sf->functionName()))->self;
	}

	return 0;
}

J3ObjectHandle* J3Thread::pendingException() {
	if(_pendingException) {
		return push(_pendingException);
	} else
		return 0;
}

void J3Thread::ensureCapacity(uint32_t capacity) {
	_localReferences.ensureCapacity(capacity);
}

J3Thread* J3Thread::nativeThread(J3ObjectHandle* handle) {
	return (J3Thread*)handle->getLong(get()->vm()->threadVMData);
}

void J3Thread::assocJavaThread(J3ObjectHandle* javaThread) {
	_javaThread = javaThread;
	_javaThread->setLong(vm()->threadVMData, (int64_t)(uintptr_t)this);
}

J3ObjectHandle* J3Thread::push(J3ObjectHandle* handle) { 
	return _localReferences.push(handle); 
}

J3ObjectHandle* J3Thread::push(J3Object* obj) { 
	return _localReferences.push(obj); 
}

J3ObjectHandle* J3Thread::tell() { 
	return _localReferences.tell(); 
}

void J3Thread::restore(J3ObjectHandle* ptr) { 
	_localReferences.restore(ptr); 
}

J3Thread* J3Thread::get() { 
	return (J3Thread*)Thread::get(); 
}
