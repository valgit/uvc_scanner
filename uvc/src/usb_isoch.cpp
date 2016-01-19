/*
 Copyright (c) 2010 Max Grosse (max.grosse(at)ioctl.eu)
 
 This software is provided 'as-is', without any express or implied
 warranty. In no event will the authors be held liable for any damages
 arising from the use of this software.
 
 Permission is granted to anyone to use this software for any purpose,
 including commercial applications, and to alter it and redistribute it
 freely, subject to the following restrictions:
 
 1. The origin of this software must not be misrepresented; you must not
 claim that you wrote the original software. If you use this software
 in a product, an acknowledgment in the product documentation would be
 appreciated but is not required.
 
 2. Altered source versions must be plainly marked as such, and must not be
 misrepresented as being the original software.
 
 3. This notice may not be removed or altered from any source
 distribution.
*/
#include "usb_isoch.h"
namespace usb {
#if 0
	const size_t COND_THRES = 8;
#endif
	
	void isoch_request::iso_callback(void* refcon, IOReturn result, void* arg0) {
		
		isoch_request *req = (isoch_request*)refcon;
		isoch_transfer *transfer = req->transfer;
		
		req->done = true;
		//uint64_t	latest_frame_num = req->frame_num;
		//printf("CALLBACK processing %llu / %llx\n",req->frame_num,(uint64_t)req);
		if(req->ZOMBIE) {
			//printf("STILL ALIIIVE!\n");fflush(stdout);
			/* ignore this packet */
			return;
		}
		
		// no longer outstanding
		transfer->req_outstanding_.erase(req);
#if 0
		pthread_mutex_lock(&(transfer->mutex_unprocessed_));
		transfer->req_done_.insert(req);
		if(transfer->req_done_.size() > COND_THRES) {
			pthread_mutex_lock(&(transfer->mutex_data_avail_));
			pthread_cond_signal(&(transfer->cond_data_avail_));
			pthread_mutex_unlock(&(transfer->mutex_data_avail_));
		}
		pthread_mutex_unlock(&(transfer->mutex_unprocessed_));
#endif
		transfer->req_done_->push(req);
		
		// no longer outstanding
		//transfer->req_outstanding_.erase(req);
		
		if(transfer->should_stop_) {
			//if(transfer->req_outstanding_.empty()) {
			if(transfer->req_outstanding_.size()==0) {
				/*** TODO: this is not hit???? ***/
				CFRunLoopStop(CFRunLoopGetCurrent());
			} else {
				//printf("CALLBACK->still %lu requests to go...\n",transfer->req_outstanding_.size());
				request_set::iterator iter = transfer->req_outstanding_.begin();
				for(;iter!=transfer->req_outstanding_.end();++iter) {
					isoch_request *ir = *iter;
					//printf("## frame_num=%llu\n",ir->frame_num);
				}
			}
		} else {
			isoch_request *nreq = new isoch_request(transfer);
			transfer->req_outstanding_.insert(nreq);
		}
	}

	/* *isoch_requst* */
	isoch_request::isoch_request(isoch_transfer* tf)
	: transfer(tf), done(false), ZOMBIE(false)
	{
		/** allocate memory stuff */
		frame_list = new IOUSBIsocFrame[transfer->num_muframes_];
		buffer = new uint8_t[transfer->pipe_size_ * transfer->num_muframes_];
		//memset(buffer, 0, transfer->pipe_size_*transfer->num_muframes_);

		for(int i=0; i<transfer->num_muframes_; ++i) {
			frame_list[i].frStatus = 0;
			frame_list[i].frReqCount = transfer->pipe_size_;
			frame_list[i].frActCount = 0;
		}
		/* launch */
		retry:
		kern_return_t ret = transfer->intf_->read_isoch(1, buffer, transfer->current_frame_,
				transfer->num_muframes_, frame_list, isoch_request::iso_callback, this);
		if(ret) {
			if(ret==kIOUSBPipeStalled) {
				printf("STALL, YUCK!\n");
				transfer->intf_->clear_stall_both_ends(1);
				goto retry;
			} else if(ret==kIOReturnIsoTooOld) {
				printf("TOO OLD, YUCK!\n");
				transfer->current_frame_ = transfer->intf_->bus_frame_number() + 50;
				goto retry;
			} else {
				printf("WTF? %x=%s\n",ret, iokit_error_to_string(ret));
				throw exception("EPIC FAIL", ret);
			}
		}
		frame_num = transfer->current_frame_;
		transfer->current_frame_ += transfer->num_frames_; 
	}
	isoch_request::~isoch_request() {
		if(!done) {
			printf("**** TRYING TO DELETE REQUEST WHICH HAS NOT HAVE IT'S CALLBACK *****\n");
			printf("**** AT: 0x%llx\n ****\n",(uint64_t)this);
			fflush(stdout);
		}
		//printf(">>> DELETING ISOCH_REQUEST %llx\n",(uint64_t)this);
		delete [] frame_list;
		delete [] buffer;
		transfer = NULL;
	}

	/* *isoch_transfer* */
	isoch_transfer::isoch_transfer(interface *intf, uint32_t pipe_size)
	: intf_(intf), pipe_size_(pipe_size), running_(false) {
		/* assume, alternate is already set correctly!!! */

		/* globals */
		num_frames_ = 5;
		num_muframes_ = num_frames_*8;
		num_reqs_ = 10;

		req_done_ = new request_rb(100,5);
		
#if 0		
		/* init global mutex on unprocessed list */
		if(pthread_mutex_init(&mutex_unprocessed_, NULL)) {
			throw exception("failed initializing mutex");
		}
		
		if(pthread_mutex_init(&mutex_data_avail_, NULL)) {
			throw exception("failed initializing mutex");
		}
		if(pthread_cond_init(&cond_data_avail_, NULL)) {
			throw exception("failed initializing condition");
		}
#endif
	}

	isoch_transfer::~isoch_transfer() {
#if 0
		pthread_mutex_lock(&mutex_unprocessed_);
		pthread_mutex_lock(&mutex_data_avail_);
#endif
		/* clear outstanding */
		if(!req_outstanding_.empty()) {
			printf("** There where still outstanding requests (should NEVER happen)?? --> %lu\n",req_outstanding_.size());
			for(request_set::iterator i=req_outstanding_.begin(); i!=req_outstanding_.end(); ++i) {
				isoch_request *req = *i;
				//delete req;
				/*** NOTE: This results in a LEAK but because of some WEIRD behavior,
				 everything is going to crash if we delete it here... ?? ***/
				req->ZOMBIE = true;
			}
			req_outstanding_.clear();
		}
#if 0
		if(!req_done_.empty()) {
			printf("** There where still unprocessed requests??\n");
			for(request_set::iterator i=req_done_.begin(); i!=req_done_.end(); ++i) {
				isoch_request *req = *i;
				delete req;
			}
			req_done_.clear();
		}

		pthread_mutex_destroy(&mutex_unprocessed_);
		pthread_mutex_destroy(&mutex_data_avail_);
		pthread_cond_destroy(&cond_data_avail_);
#endif
		delete req_done_;
	}

	void isoch_transfer::start() {
		/* launch new thread */
		pthread_attr_t attr;		
		pthread_attr_init(&attr);
		pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

		if(running_) {
			printf("ALREADY RUNNING!\n");
			return;
		}
		running_ = true;
		should_stop_ = false;
		if(pthread_create(&worker_thread_, &attr, isoch_transfer::worker_func, (void*)this)) {
			printf("Thread creation FAILED (isoch_transfer)\n");
			running_ = false;
		}
	}

	void isoch_transfer::stop() {
		intf_->abort_pipe(1);
		usleep(50*1000);
		//printf("TRANSFER GOING DOWN...waiting for worker_thread to join...\n");
		should_stop_ = true;
		pthread_join(worker_thread_, NULL);
		//printf("TRANSFER GOING DOWN...JOINED\n");
		running_ = false;
#if 0
		/* signal decoder to finish */
		pthread_mutex_lock(&(mutex_data_avail_));
		pthread_cond_signal(&(cond_data_avail_));
		pthread_mutex_unlock(&(mutex_data_avail_));
#endif
	}

	void* isoch_transfer::worker_func(void* vme) {
		isoch_transfer *me = (isoch_transfer*)vme;
		//printf("ENTER worker_func, isoch_transfer=0x%llx\n", (uint64_t)me);

		me->req_done_->begin();
		
		/* cf run loop set up */
		CFRunLoopSourceRef		source;
		me->intf_->async_event_source(&source);	
		CFRunLoopAddSource( CFRunLoopGetCurrent(), source, kCFRunLoopDefaultMode );
		/* transfer set-up */
		me->current_frame_ = me->intf_->bus_frame_number();
		// be safe:
		me->current_frame_ += 100;

		me->intf_->clear_stall_both_ends();
		/* GO and LAUNCH that transfers! */
		
		for(int i=0; i<me->num_reqs_; ++i) {
			isoch_request *req = new isoch_request(me);
			me->req_outstanding_.insert(req);
		}
	
		/* wait for callbacks */
		//CFRunLoopRun();
		while(!me->should_stop_) {
			CFRunLoopRunInMode(kCFRunLoopDefaultMode, 2, false);
		}
		/* done */
		CFRunLoopStop(CFRunLoopGetCurrent());
		CFRunLoopRemoveSource( CFRunLoopGetCurrent(), source, kCFRunLoopDefaultMode );

#if 0
		/* better signal 'em */
		// beter signal decoder threads
		pthread_cond_signal(&(me->cond_data_avail_));
		pthread_mutex_unlock(&(me->mutex_data_avail_));
#endif
		me->req_done_->end();
		
		return NULL;
	}
#if 0
	/** start_process. Requests permission to
	 * process all returned requests.
	 * After calling this, you should as fast as
	 * possible call end_process() so processing
	 * can continue. best to first copy/move all stuff
	 * **/
	request_set* isoch_transfer::start_process() {
		pthread_mutex_lock(&mutex_unprocessed_);
		return &req_done_;
	}

	void isoch_transfer::end_process() {
		pthread_mutex_unlock(&mutex_unprocessed_);
	}

	void isoch_transfer::wait_for_data() {
		if(!running_) return;
		pthread_mutex_lock(&mutex_data_avail_);
		pthread_cond_wait(&cond_data_avail_, &mutex_data_avail_);
		pthread_mutex_unlock(&mutex_data_avail_);
	}
#endif
	
}

