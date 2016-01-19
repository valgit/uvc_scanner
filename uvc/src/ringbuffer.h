/* ******************************************************************

Copyright (c) 2010 max.grosse < max . grosse {at} ioctl . eu >

This  software is  provided 'as-is', without  any express or  implied
warranty.  In  no event  will  the  authors  be  held liable for  any
damages arising from the use of this software.

Permission  is  granted  to  anyone  to  use  this  software for  any
purpose,  including  commercial applications, and  to  alter  it  and
redistribute  it  freely,  subject  to  the  following  restrictions:

    1. The  origin  of  this  software  must not  be  misrepresented;
    you  must not claim that you wrote the original software.  If you
    use this software in a product,  an acknowledgment in the product
    documentation would be appreciated but is not required.

    2. Altered source  versions must be  plainly marked as such,  and
    must not be misrepresented as being the original software.

    3. This notice  may not  be removed or  altered  from  any source
    distribution.

****************************************************************** */ 

/** ringbuffer.h
 * Class for multithreaded ringbuffer:
 * A producer thread writes to the buffer, a consumer thread reads
 * from the buffer.
 * Designed to result in NO POLLING as well as the LEAST BLOCKING
 * necessary.
 *
 * There is an internal wrap-around once 2^64 entries have been
 * added. Having not waited for so long, i can not guarentee that
 * everything there works (at all...) ===> TODO FIXME
 *
 * NOTE that entries dropped (because consumer is too slow) will
 * get delete-ed. If this is not desired, provide a different
 * ringbuffer_delete which does not do anything ;)
 *
 * First version: Mi 10 Nov 2010 17:57:43 CET
 *
 * Sample usage:
---------------------------->8--------------

#include <ringbuffer.h>

void* producer(void* ctx) {
	ringbuffer<int> &rb = *(ringbuffer<int>*)ctx;
	rb.begin();
	for(int i=0; i<60; ++i) {
		printf("[prod] add %d\n", i);
		rb.push( i );
		usleep(10 * 1000); /// to simulate processing time
	}
	rb.end();
	return NULL;
}

void* consumer(void* ctx) {
	ringbuffer<int> &rb = *(ringbuffer<int>*)ctx;
	rb.begin();
	while(!rb.done()) {
		int v = rb.pop();
		printf("[cons] eat %d\n", v);
		usleep(100 * 1000); /// to simulate processing time
	}
	rb.end();
	return NULL;
}

---------------------------->8--------------
 * 
 **/

#ifndef RINGBUFFER_H__
#define RINGBUFFER_H__
#include <pthread.h>
#include <cstdio>
//#include <tr1/cstdint>
#include <stdint.h>
const uint64_t __MAX_UINT64 = 1ULL<<63;

template<typename TYPE>
struct ringbuffer_deleter {
void operator() (TYPE t) { delete t; } };

#define __BASE_TYPE_DELETER(T) \
template <> \
struct ringbuffer_deleter<T> { \
void operator() (T t) {}}
__BASE_TYPE_DELETER(int8_t);
__BASE_TYPE_DELETER(int16_t);
__BASE_TYPE_DELETER(int32_t);
__BASE_TYPE_DELETER(int64_t);
__BASE_TYPE_DELETER(uint8_t);
__BASE_TYPE_DELETER(uint16_t);
__BASE_TYPE_DELETER(uint32_t);
__BASE_TYPE_DELETER(uint64_t);

__BASE_TYPE_DELETER(char);
__BASE_TYPE_DELETER(bool);
__BASE_TYPE_DELETER(float);
__BASE_TYPE_DELETER(double);
#undef __BASE_TYPE_DELETER

template<typename TYPE, typename DELETER = ringbuffer_deleter<TYPE> >
class ringbuffer {
	public:
		typedef TYPE type;

		ringbuffer(size_t capacity, size_t tol, DELETER del = DELETER())
		: capacity_(capacity), thres_(tol), del_(del)
		{
			buffer_ = new type[capacity];
			head_ = 0ULL;
			tail_ = 0ULL;
			signal_at_ = head_+thres_;
			done_ = false;

			pthread_cond_init(&cond_, NULL);
			pthread_mutex_init(&mutex_, NULL);
			pthread_mutex_init(&overflow_mutex_, NULL);
		}
		virtual ~ringbuffer() {
			/* clean any remaining elements as well */
			for(size_t i=tail_; i<head_; ++i) {
				del_(buffer_[i%capacity_]);
			}
			delete [] buffer_;
			pthread_mutex_destroy(&mutex_);
			pthread_cond_destroy(&cond_);
			pthread_mutex_destroy(&overflow_mutex_);
		}

		template<typename R>
		void push(R& ival) {
			type val = static_cast<type>(ival);
			if(head_-tail_>=capacity_) {
				printf("PUSH: OVERRUN!\n");
				del_(val);
				return;
			}
			buffer_[head_%capacity_] = val;
			head_++;
			if(head_>=signal_at_) {
				//printf("SIGNALLING\n");
				signal_at_ = __MAX_UINT64;
				pthread_cond_signal(&cond_);
				pthread_mutex_unlock(&mutex_);
			}
			check_overflow();
		}
		type pop() {
			pthread_mutex_lock(&overflow_mutex_);
			uint64_t HEAD = head_; /// read once!

			/* too close? */
			if(tail_==head_) {
				signal_at_ = tail_+thres_;
				/* wait... */
				//printf("waiting: %llu\n",signal_at_);
				pthread_cond_wait(&cond_, &mutex_);
			}

			while(HEAD-tail_>=2*thres_) {
				printf("POP: UNDERRUN\n");
				//delete buffer_[tail_%capacity_];
				del_(buffer_[tail_%capacity_]);
				tail_ ++;
			}

			size_t tmp = tail_%capacity_;
			tail_ ++;
			pthread_mutex_unlock(&overflow_mutex_);
			return buffer_[tmp];
		}

		void begin() {
			pthread_mutex_lock(&mutex_);
		}
		void end() {
			done_ = true;
			pthread_cond_signal(&cond_);
			pthread_mutex_unlock(&mutex_);
		}

		bool done() const { return done_; }

	private:
		size_t					capacity_;
		size_t					thres_;

		type					*buffer_;
		uint64_t				head_;
		uint64_t				tail_;

		uint64_t				signal_at_;
		pthread_cond_t			cond_;
		pthread_mutex_t			mutex_;
		pthread_mutex_t			overflow_mutex_;

		bool					done_;

		DELETER					del_;

		inline void check_overflow() {
			if(head_>=__MAX_UINT64-1) {
				pthread_mutex_lock(&overflow_mutex_);
				head_ %= capacity_;
				tail_ %= capacity_;
				pthread_mutex_unlock(&overflow_mutex_);
			}
		}

};

#endif /* RINGBUFFER_H__ */

