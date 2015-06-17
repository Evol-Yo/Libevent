/*
 * Copyright (c) 2000-2007 Niels Provos <provos@citi.umich.edu>
 * Copyright (c) 2007-2012 Niels Provos and Nick Mathewson
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#ifndef _EVENT2_EVENT_STRUCT_H_
#define _EVENT2_EVENT_STRUCT_H_

/** @file event2/event_struct.h

  Structures used by event.h.  Using these structures directly WILL harm
  forward compatibility: be careful.

  No field declared in this file should be used directly in user code.  Except
  for historical reasons, these fields would not be exposed at all.
 */

#ifdef __cplusplus
extern "C" {
#endif

#include <event2/event-config.h>
#ifdef _EVENT_HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef _EVENT_HAVE_SYS_TIME_H
#include <sys/time.h>
#endif

/* For int types. */
#include <event2/util.h>

/* For evkeyvalq */
#include <event2/keyvalq_struct.h>

//事件标志(ev_flags)
#define EVLIST_TIMEOUT	0x01
#define EVLIST_INSERTED	0x02
#define EVLIST_SIGNAL	0x04
#define EVLIST_ACTIVE	0x08
#define EVLIST_INTERNAL	0x10
#define EVLIST_INIT	0x80

/* EVLIST_X_ Private space: 0x1000-0xf000 */
#define EVLIST_ALL	(0xf000 | 0x9f)

/* Fix so that people don't have to run with <sys/queue.h> */
#ifndef TAILQ_ENTRY
#define _EVENT_DEFINED_TQENTRY
#define TAILQ_ENTRY(type)						\
struct {								\
	struct type *tqe_next;	/* next element */			\
	struct type **tqe_prev;	/* address of previous next element */	\
}
#endif /* !TAILQ_ENTRY */

#ifndef TAILQ_HEAD
#define _EVENT_DEFINED_TQHEAD
#define TAILQ_HEAD(name, type)			\
struct name {					\
	struct type *tqh_first;			\
	struct type **tqh_last;			\
}
#endif

struct event_base;

//event - Libevent的核心-事件处理器(EventHandler)
struct event {
	//活动事件队列(不止一个),不同优先级的事件处理器将插入不同的活动事件队列
	//TAILQ_**等一系列宏参考compat/sys/queue.h
	TAILQ_ENTRY(event) ev_active_next; 
	//所有已注册事件处理器(包括I/O、信号)
	TAILQ_ENTRY(event) ev_next; 
	
	//定时事件处理器(定时器)(一个定时器是否是通用定时器取决于其超时值大小，
	//具体参考event.c中is_common_timeout())
	union {
		//对于通用定时器队列(采用链表)，该值指出了该定时器在链表中的位置
		TAILQ_ENTRY(event) ev_next_with_common_timeout;
		//对于其他定时器(采用minheap)，该值指出了该定时器在minheap中的位置
		int min_heap_idx;
	} ev_timeout_pos; 
	
	//通用描述符，对于I/O事件，它是文件描述符值，对于信号事件，它是信号值
	evutil_socket_t ev_fd;
	//该事件处理器的event_base实例
	struct event_base *ev_base;

	//程序中，我们可能对同一个socket文件描述符上的可读可写事件创建多个事件处理器
	//(具有不同的回调函数)，Libevent将具有相同文件描述符的事件处理器组织在一起
	union {
		/* used for io events */
		struct {
			TAILQ_ENTRY(event) ev_io_next; //IO事件队列
			struct timeval ev_timeout;
		} ev_io;

		/* used by signal events */
		struct {
			TAILQ_ENTRY(event) ev_signal_next; //信号事件队列
			short ev_ncalls;
			/* Allows deletes in callback */
			short *ev_pncalls;
		} ev_signal;
	} _ev; 

	//事件类型(如上)
	short ev_events;
	short ev_res;		/* result passed to event callback */
	short ev_flags;     	//事件标志
	ev_uint8_t ev_pri;		//事件处理器优先级，值越小优先级越高
	ev_uint8_t ev_closure; //指定event_base执行事件处理器的回调函数的行为(event-internal.h)
	struct timeval ev_timeout; //仅对定时器有效，指定定时器的超时值

	//事件处理器的回调函数，由event_base调用。回调函数调用时，
	//ev_fd、ev_res和ev_arg被传入该回调函数
	void (*ev_callback)(evutil_socket_t, short, void *arg);
	void *ev_arg;
};

TAILQ_HEAD (event_list, event);

#ifdef _EVENT_DEFINED_TQENTRY
#undef TAILQ_ENTRY
#endif

#ifdef _EVENT_DEFINED_TQHEAD
#undef TAILQ_HEAD
#endif

#ifdef __cplusplus
}
#endif

#endif /* _EVENT2_EVENT_STRUCT_H_ */
