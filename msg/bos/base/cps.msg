$ ENGLISH_SOURCE_VERSION= 1.4
$ @(#)99	1.3  src/msg/bos/base/cps.msg, bosmsg, msg411, 9430C411a 7/23/94 12:21:56
$
$   COMPONENT_NAME: libncs
$
$   FUNCTIONS: 
$
$   ORIGINS: 84
$
$
$ -----------------------------------------------------------------------------
$
$ cps.msg
$ 
$ Message source object file used by the gencat utility for message catalogue
$ generation.  
$
$ -----------------------------------------------------------------------------
$						
$quote "
$delset 5
$set 5
$
$ -------------------------- timer handler ------------------------------------
$
$
$ The next 6 messages are DEVELOP messages
$
$  Errors from toggling interval timer
1  "(task__set_timeslice) panic. errno=%%1$d\n"
2  "(set_ec_timeslice) panic. errno=%%1$d\n"
3  "(task__os_yield) panic: task did not yield; sigmask=0x%%1$x\n"
4  "(sigaddblock) called, new=%%1$x sig=%%2$d mask=%%3$x\n"
5  "(sigremblock) called, old=%%1$x sig=%%2$d mask=%%3$x\n"
6  "gf_open:"
$
$ --------------------------- I/O handler -------------------------------------
$
$ The strings in this routine are all DEBUG statements.
$ The next 8 messages are NOTRANS.
7  "**error**"
$
$ name and number of port
8  "%1$s[%2$ld]"
9  "[%08x, "
10  "[null, "
11  "%08x, "
12  "null, "
13  "%08x]"
14  "null]"
$
$ The next 13 messages are DEVELOP.
15  "(pfm__init) called, task=%%1$s[%%2$lx] flags=%%3$lx\n"
16  "DT"
17  "(pfm__enable) can't deliver signal because inhibited inh_cnt=%%1$d flt_cnt=%%2$d hard_cnt=%%3$d\n"
18  "(pfm__hard_enable) called, task=%%1$s[%%2$lx] inh_cnt=%%3$d flt_cnt=%%4$d new_hard_cnt=%%5$d\n"
19  "(pfm__hard_enable) can't deliver signal because inhibited inh_cnt=%%1$d flt_cnt=%%2$d hard_cnt=%%3$d\n"
20  "(pfm__inhibit_faults) called, task=%%1$s[%%2$lx] inh_cnt=%%3$d new_flt_cnt=%%4$d hard_cnt=%%5$d\n"
21  "(pfm__enable_faults) called, task=%%1$s[%%2$lx] inh_cnt=%%3$d new_flt_cnt=%%4$d hard_cnt=%%5$d\n"
22  "(pfm__enable_faults) can't deliver signal because inhibited inh_cnt=%%1$d flt_cnt=%%2$d hard_cnt=%%3$d\n"
23  "(pfm___cleanup) called, task=%%1$s[%%2$lx] st=%%3$lx crec=%%4$lx\n"
24  "(pfm___rls_cleanup) called, task=%%1$s[%%2$lx] crec=%%3$lx\n"
25  "(pfm___rls_cleanup) bad release order, task=%%1$s[%%2$lx] crec=%%3$lx\n"
26  "(pfm___reset_cleanup) called, task=%%1$s[%%2$lx] crec=%%3$lx\n"
27  "(pfm__signal) called, task=%%1$s[%%2$lx] st=%%3$lx\n"
$
$ ADMIN
28  "*** Exiting: exception caught by PFM system cleanup handler\n"
$
$ The next 10 messages are DEVELOP
29  "(pfm__signal) about to unwind, crec %%1$#x st %%2$#x\n"
30  "(pfm__exit) called, task=%%1$s[%%2$lx]\n"
31  "DT"
32  "(mutex__lock) called, task=%%1$s[%%2$lx] mutex=%%3$lx timeout=%%4$lx:%%5$lx\n"
33  "mutex_lock) panic ! ATOMIC_INC failed !\n"
34  "(mutex__lock) got lock\n"
35  "(mutex__lock) missed lock, owned=%%1$d owner=%%2$lx\n"
36  "(mutex__unlock) called, task=%%1$s[%%2$lx] mutex=%%3$lx\n"
37  "(rec_mutex_lock) called, task=%%1$s[%%2$lx] rec_mutex=%%3$lx\n"
38  "(rec_mutex_unlock) called, task=%%1$s[%%2$lx] rec_mutex=%%3$lx\n"
$
$ ------------------------- Task Management -----------------------------------
$ The next 16 messages are DEVELOP
$
39  "***Error (add_task_to_list) which_list out of bounds\n"
40  "(add_task_to_list) Panic ! Called while not inhibited\n"
41  "(add_task_to_list) resulting state:\n"
42  "(get_tcp_p) Panic ! Called while not inhibited\n"
43  "(get_tcb_p) handle has bad index: %%1$lx\n"
44  "(get_tcb_p) stale task handle: %%1$lx\n"
45  "(get_tcb_p) bad task handle: %%1$lx\n"
46  "(get_tcb_p) task handle already reused: %%1$lx\n"
47  "(remove_task_from_list) Panic ! Called while not inhibited\n"
48  "***Error (remove_task_from_list) which_list out of bounds\n"
49  "(remove_task_from_list) task_ptr=%%1$x task %%2$s[%%3$lx] sigmask=%%4$x\n"
50  "(remove_task_from_list) resulting state:\n"
51  "%%1$s handle=%%2$lx prev=%%3$x tcb=%%4$x next=%%5$x name=%%6$s\n"
52  "(task__base_proc) enter\n"
53  "(task__base_proc) Entered with non-zero inhibit count\n"
54  "(task__base_proc) running cleanup handler for task %%1$s[%%2$lx]\n"
$
$ The next 2 messages are ADMIN.
$
$ %1 is the name of the task, %2 is this task's handle,
$ %3 is the code explaining this problem
55  "Unhandled exception in task %%1$s[%%2$lx], status = %%3$lx\n"
56  "(task__base_proc) inhibit screwup !\n"
$
$ The next 15 messages are DEVELOP. 
$
57  "(task__base_proc) task %%1$s[%%2$lx] exiting\n"
58  "(task__blast) called, handle: %%1$lx\n"
59  "(task__blast) invalid handle: %%1$lx\n"
60  "(task__blast) handle: %%1$lx, can't blast yourself\n"
61  "(task__blast) can't blast DT\n"
62  "(task__blast) Removing task from ready q\n"
63  "(task__blast) Removing task from wait q\n"
64  "(task__block) called, task %%1$s[%%2$lx]\n"
65  "(task__block) task %%1$s[%%2$lx] already blocked\n"
66  "(task__block) resulting state:\n"
67  "(task__block) task %%1$s[%%2$lx] unblocked\n"
68  "(task__create) called, pri:%%1$ld\n"
69  "(task__create) invalid priority: %%1$ld\n"
70  "(task__create) invalid lifetime: %%1$d\n"
71  "(task__create) can't allocate %%1$d bytes for stack\n"
$
$ NOTRANS
72  "\0"
$
$ The next 13 messages are DEVELOP.
$
73  "(task__create) out of task slots\n"
74  "(task__create) task handle=%%1$lx and tcb ptr=%%2$x\n"
75  "(task__create) initializing context\n"
76  "(task__create) context initialized\n"
77  "(task__exit) called\n"
78  "(task__exit) can't exit from DT\n"
79  "(task__get_ec) called, handle=%%1$lx ec=%%2$x\n"
80  "(task__get_ec) invalid handle: %%1$lx\n"
81  "(task__get_ec) no completion EC, handle: %%1$lx\n"
82  "(task__get_uuid) called\n"
83  "(task__get_handle) called\n"
84  "(task__get_info) called, handle=%%1$lx\n"
85  "(task__get_info) invalid handle: %%1$lx\n"
$
$ ADMIN message.
86  "(task__sched): No runnable tasks.\n"
$
$ The next 9 messages are DEVELOP.
87  "(task__set_name) called, handle=%%1$lx name=%%2$.*s\n"
88  "(task__set_name) invalid handle: %%1$lx\n"
89  "(task__set_name) invalid name length: %%1$ld\n"
90  "(task__set_result) called, compl_status=%%1$08lx, outval=%%2$ld\n"
91  "(task__set_result) no task completion EC, handle: %%1$lx\n"
92  "(task__release) called, handle=%%1$lx compl=%%2$lx, out=%%3$ld\n"
93  "(task__release) invalid handle: %%1$lx\n"
94  "(task__release) no completion EC, handle: %%1$lx\n"
95  "(task__release) task not completed, handle: %%1$lx\n"
$
$ ADMIN message.
96  "(task__resched) entry state:\n"
$
$ DEVELOP message.
97  "(task__resched) task switch %%1$20s[%%2$lx] ==> %%3$20s[%%4$lx]\n"
$
$ ADMIN message.
98  "(task__resched) resulting state:\n"
$
$ The next 12 messages are DEVELOP.
$
99  "(task__resched) state after resume task %%1$s[%%2$lx] state:\n"
100  "(task__resched) delivering pending cleanup\n"
101  "(task__resched) can't deliver signal because inhibited\n"
102  "(task__signal) called, handle=%%1$lx fault=%%2$lx\n"
103  "(task__signal) invalid handle: %%1$lx\n"
104  "(task__signal) signalling %%1$s[%%2$lx] with status %%3$lx\n"
105  "(task__terminate_system) called, status=%%1$lx\n"
106  "(task__wakeup) called, handle=%%1$lx in_status=%%2$lx\n"
107  "(task__wakeup) invalid handle: %%1$lx\n"
108  "(task__wakeup) waking task %%1$s[%%2$lx]\n"
109  "(task__wakeup) task %%1$s[%%2$lx] already awake\n"
$
$ %1 is the name of the task, %2 is this task's handle,
$ %3 is the priority of the task
110  "Scrambled q, task %%1$s[%%2$lx] priority=%%3$d \n"
$
$ ADMIN message.
111  "(task__wakeup) resulting state:\n"
$
$ The next 6 messages are DEVELOP.
$
112  "(task__yield) called\n"
113  "***Error: wrong state, task:%%1$d q:%%2$d\n"
114  "***Error: wrong priority, task:%%1$d q:%%2$d\n"
115  "***Error: circular list error\n"
$
$ %l is name of file, %2 is line in that file, 
$ %3 tells us if we're in the signal handler, %4 is the number of active tasks
$ %5 is the number of queued tasks
116  "***Error (%%1$s:%%2$d): invalid task lists%%3$s, active:%%4$d != queued:%%5$d\n"
117  " in signal handler"
