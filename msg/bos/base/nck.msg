$ ENGLISH_SOURCE_VERSION= 1.4
$ @(#)01        1.3  src/msg/bos/base/nck.msg, bosmsg, msg411, 9430C411a 7/23/94 12:25:18
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
$ nck.msg
$
$ Message source object file used by the gencat utility for message catalogue
$ generation.  It is a compilation of catalogues messages that were extracted 
$ from the souce code modules that comprise the following utilities:
$
$						lb_find
$						llbd
$						mkedb
$						nllbd
$						stcode
$						uuid_gen
$
$ -----------------------------------------------------------------------------
$
$quote "
$delset 3
$set 3
$
$ --------------------------------- Error Processing --------------------------
$
$ The first 3 messages are USER.
$
1  "status %%1$lx"
$
$ %1 is the status code, %2 is the subsystem
2  "status %%1$lx (%%2$s)"
$
$ %1 is the status code, %2 is the subsystem, %3 is the module name
3  "status %%1$lx (%%2$s/%%3$s)"
$
$ %1 is the error text, %2 is the subsystem, %3 is the module name. NOTRANS.
4  "%%1$s (%%2$s/%%3$s)"
$ 
$ The next 275 messages are DEVELOP.
5  "OS"
6  "time manager"
7  "no timer queue entry"
8  "entry to be cancelled not found"
9  "quit while waiting for event"
10  "bad timer interrupt"
11  "bad timer key"
12  "alarm fault"
13  "real interval timer fault"
14  "virtual interval timer fault"
15  "queue element not in use"
16  "queue element not found"
17  "cpu time limit exceeded"
18  "time adjustment out of range"
19  "queue element already in use"
20  "relative time is too large"
21  "naming server"
22  "directory is full"
23  "name already exists"
24  "invalid pathname"
25  "invalid link"
26  "not a link"
27  "name not found"
28  "invalid link operation"
29  "invalid leaf"
30  "node is unavailable"
31  "bad directory"
32  "branch is not a directory"
33  "directory is not empty"
34  "name is not a file"
35  "illegal directory operation"
36  "bad type"
37  "no rights"
38  "insufficient rights"
39  "unable to delete system bootstrap (sysboot)"
40  "directory is in use (locked)"
41  "name server helpers clocks are skewed"
42  "illegal request made of name server helper"
43  "cannot find entry in replicated root"
44  "last entry in replicated root returned"
45  "name server helper is shutdown"
46  "name server helper sent packet with errors"
47  "clocks skewed"
48  "cant find name server helper"
49  "directory must be root"
50  "directory not found in pathname"
51  "too many components in pathname"
52  "cache entry is stale"
53  "cache entry was stale and was updated"
54  "name server helper is uninitialized"
55  "internal error"
56  "bad request header version"
57  "bad request code"
58  "bad request body version"
59  "request ignored"
60  "directories not neighbors"
61  "directory not local"
62  "pathname truncated"
63  "leaf truncated"
64  "bad buffer size"
65  "object is not an acl object"
66  "volume has been mounted read-only"
67  "write-protected volume prevents recovery of a damaged directory"
68  "too many hard links exist to file"
69  "directory object not found"
70  "ran out of address space"
71  "file server"
72  "object not found"
73  "object is remote"
74  "bad reply received from remote node"
75  "communications problem with remote node"
76  "object is not locked by this process"
77  "object is in use"
78  "illegal lock request"
79  "lock violation detected"
80  "local lock table is full"
81  "remote lock table is full"
82  "operation cannot be done from here"
83  "no more lock table entries"
84  "volume uid is unavailable"
85  "locking files is blocked for this volume"
86  "locking is already blocked for this volume"
87  "wrong type - can't operate on system objects"
88  "objects are on different volumes"
89  "invalid argument"
90  "incompatible request"
91  "fault handler"
92  "odd address error"
93  "illegal instruction"
94  "integer divide by zero"
95  "CHK instruction trapped - value out of range?"
96  "arithmetic overflow"
97  "privileged instruction violation"
98  "invalid SVC code"
99  "invalid SVC procedure name"
100  "undefined TRAP instruction"
101  "unimplemented instruction"
102  "protection boundary violation"
103  "bus time-out"
104  "invalid user stack pointer"
105  "correctable memory error detected"
106  "uncorrectable memory error detected"
107  "process quit"
108  "access violation"
109  "CPU B enabled with MMU valid bit reset"
110  "null process running on CPU B"
111  "OS-internal quit (with display return)"
112  "single step completed"
113  "invalid user-generated fault (subsystem code = 0)"
114  "fault in user-space interrupt handler for pbu device"
115  "process stop"
116  "process BLAST"
117  "cache parity error"
118  "PEB WCS parity error"
119  "unimplemented SVC"
120  "invalid stack format"
121  "memory parity error"
122  "process interrupt"
123  "supervisor fault while resource lock(s) set"
124  "spurious parity error"
125  "floating point inexact result (loss of significance)"
126  "floating point divide by zero"
127  "floating point underflow"
128  "floating point operand error"
129  "floating point overflow"
130  "process suspend fault"
131  "process suspend from keyboard"
132  "process suspend due to background read"
133  "process suspend due to background write"
134  "process continue fault"
135  "fault(s) lost; process suspended or pfm__enable/pfm__inhibit mismatch?"
136  "coprocessor protocol violation"
137  "floating point branch/set on unordered condition"
138  "floating point signalling not-a-number"
139  "invalid thread during parity error check"
140  "illegal page fault in user gpio interrupt routine"
141  "bus error while running on cpu B"
142  "spurious interrupt"
143  "unexpected bus error during system initialization"
144  "cleanup handler set"
145  "cleanup handler released out of order"
146  "ac power failure"
147  "fpx parity error"
148  "unknown fpa exception"
149  "VME bus error on bus error"
150  "AT bus parity error -- (I/O channel chk)"
151  "Breakpoint"
152  "Translate Error"
153  "Illegal Lock"
154  "Lock Timeout"
155  "Unknown Status"
156  "Floating Point Exception"
157  "A88K FPU Internal Error: Specific code in FPCSR<6:0>"
158  "pause interrupt received by cpu"
159  "halt interrupt received by cpu"
160  "trap occurred while already in trap mode"
161  "unaligned instruction"
162  "floating point hardware error -- incorrect data stored to memory"
163  "integer processor hardware error -- incorrect data stored to memory"
164  "level 2 eventcount manager"
165  "internal table exhausted"
166  "asynchronous fault occurred while waiting"
167  "bad eventcount"
168  "unable to allocate level 1 eventcount"
169  "level 1 eventcount not allocated"
170  "display manager"
171  "DM process"
172  "Command syntax error"
173  "Unknown command name"
174  "Too many command arguments"
175  "Position specification syntax error"
176  "Key definition incomplete"
177  "File not found"
178  "Unable to access file"
179  "Text is read-only"
180  "No text under cursor"
181  "Too many windows"
182  "Too many pads"
183  "Display manager internal error #1"
184  "Unable to write new file"
185  "You must finish current command input request before making another"
186  "Key definition too long"
187  "Cursor is not in any window"
188  "Line too long - truncated"
189  "CC command requires position mark"
190  "Unable to create new process"
191  "Unable to create new pad"
192  "Input pads cannot be replicated"
193  "Bad font file"
194  "Font table full"
195  "Unknown function key name"
196  "Window too small for any text"
197  "Display manager internal error #2"
198  "Pad deleted by client process"
199  "Pad still active"
200  "Illegal grow coordinates"
201  "Display manager pads cannot be named"
202  "File is not an ascii file"
203  "Processes still active"
204  "Bad text range"
205  "Paste buffer empty"
206  "Invalid regular expression pattern"
207  "Pattern too long"
208  "Illegal substitution expression"
209  "No match"
210  "DM input window cannot be deleted"
211  "No room for more text at that position"
212  "Invalid hexadecimal digit"
213  "Invalid tabstop value"
214  "Edit pad already open on that file"
215  "File already modified"
216  "No window selected for grow or move"
217  "Line contains non-editable controls"
218  "Cannot delete .bak file"
219  "Heap file inconsistent during recovery"
220  "A process with that name already exists"
221  "Search aborted"
222  "Operation illegal during search or substitute"
223  "Argument value out of range"
224  "User logging out"
225  "Error processing user_data/key_defs"
226  "Previous fatal error prevents typing (try RW -R)"
227  "Nothing left to UNDO"
228  "Command illegal except during node startup"
229  "Can not create paste buffer"
230  "You cannot edit this pad"
231  "Pathname has changed -- close window and reread"
232  "Can not edit that object type"
233  "Invalid paste buffer name"
234  "Operation is illegal on an icon"
235  "Maximum file size has been exceeded"
236  "Echo mode is already on; ABRT will end echoing"
237  "Echo mode aborted"
238  "Name does not correspond to a window"
239  "Command aborted"
240  "Command is not allowed during login"
241  "Logout has been disabled"
242  "Insufficient memory for operation. Try closing pads"
243  "Paste error"
244  "Previous attempt to read file failed. Try reopening"
245  "Cannot set privileged environment variable"
246  "Variable not defined"
247  "Command illegal after login"
248  "No balancing character found"
249  "Too many open characters"
250  "Too many close characters"
251  "Current color map usage prevents turning MONO off"
252  "Mailbox data not available; pad request lost."
253  "Commands cdm and mono are mutually exclusive"
254  "Window no longer exists"
255  "Command illegal when WMGR -OFF"
256  "Command illegal in foreign windows"
257  "WARNING: Paste buffer is too large for undo buffer"
258  "Unable to save file until no one else is reading it"
259  "Command valid by super user only"
260  "Command disabled while window is offscreen"
261  "Command illegal in obscured window & DM is not window manager"
262  "Can't logout from DM.  Use 'ex' to exit DM."
263  "Invalid display unit number"
264  "-U option ignored when de-iconizing windows"
265  "Loading user preferred key defs from ~/user_data/key_defs"
266  "Insufficient rights for SHUT/EX"
267  "socket manager"
268  "address manipulation"
269  "input buffer too large"
270  "output buffer too small"
271  "bad numeric format name"
272  "can't find host name"
273  "can't convert address to host name"
274  "can't get name for local host"
275  "can't create socket for internal use"
276  "can't get network interface configuration information"
277  "address family invalid"
278  "invalid name format"
$
$ The next 310 messages are USER.
$
279  "network computing system"
280  "NCA"
281  "division by zero"
282  "address error"
283  "floating point division by zero"
284  "invalid tag in discriminated union value"
285  "invalid array bound"
286  "RPC runtime"
287  "communications failure"
288  "operation number out of range"
289  "unknown interface"
290  "can't create socket"
291  "can't bind socket"
292  "wrong boot time"
293  "too many interfaces defined"
294  "not in call"
295  "you crashed"
296  "no port specified in sockaddr or interface specification"
297  "protocol error"
298  "too many sockets allocated"
299  "invalid interface re-register"
300  "can't recv"
301  "bad packet"
302  "no location in handle"
303  "address in use"
304  "input arguments too big"
305  "output arguments too big"
306  "server is too busy to handle call"
307  "string parameter longer than declared maximum length"
308  "tried to register too many objects"
309  "invalid handle"
310  "client isn't authenticated"
311  "invalid authentication type"
312  "can't allocate heap storage"
313  "can't allocate near heap storage"
314  "locating broker"
315  "database format out of date"
316  "database in use in an incompatible manner"
317  "no matching entry available"
318  "unable to complete registration operation"
319  "can't access database"
320  "can't reach location server"
321  "bad entry"
322  "DRM"
323  "incorrect software or database version"
324  "resource locked"
325  "resource not available"
326  "data file full"
327  "propagation file full"
328  "replica unconfigured"
329  "replica uninitialized"
330  "replica deleted"
331  "replica closed"
332  "replica reset"
333  "program error"
334  "replica already exists"
335  "replica not found"
336  "too many replicas"
337  "bad replica address"
338  "bad remote replica state"
339  "earlier timestamp"
340  "no such entry"
341  "entry not found"
342  "entry already exists"
343  "bad entry length"
344  "bad record number"
345  "object already open"
346  "too many objects opened"
347  "object not open"
348  "bad argument"
349  "create failed"
350  "file io error"
351  "too many keys"
352  "object already exists"
353  "bad protocol family"
354  "no default replica specified"
355  "Registry Server"
356  "Operation not yet implemented"
357  "Operation not supported on specified domain"
358  "Entry already exists"
359  "Name already exists"
360  "Unix id changed or incorrect"
361  "Entry is an alias"
362  "No more matching entries"
363  "Entry not found"
364  "Registry server unavailable"
365  "Person is not a member of specified group"
366  "Person is not a member of specified org"
367  "Person is not a member of specified group and org"
368  "Incomplete account name"
369  "Password incorrect"
370  "User not authorized to perform operation"
371  "Registry is read only or operation performed at a slave"
372  "PGO alias entry has invalid owner"
373  "Invalid data - record too long"
374  "Registry Server Replication"
375  "Program error"
376  "Invalid database version"
377  "Unable to create database"
378  "Unable to open database"
379  "Unable to read database"
380  "Unable to save database"
381  "Unable to rename database"
382  "Master registry site not found"
383  "Registry site is the master"
384  "Registry site is a slave"
385  "Update is not from master"
386  "Replica is in a bad state"
387  "Replica name is unavailable"
388  "Replica has bad address"
389  "Replica is in the deleted state"
390  "Database is locked"
391  "Clocks are skewed"
392  "Update arrived out of sequence"
393  "Initialization partner unavailable"
394  "RPROC"
395  "Can't run specified program"
396  "Can't create a new process for program"
397  "Server internal error"
398  "Too many arguments specified"
399  "Server won't create a process"
400  "Server can't change identity of new process"
401  "Kerberos"
402  "Principal expired"
403  "Service expired"
404  "Authentication expired"
405  "Unknown protocol version number"
406  "Principal: Incorrect master key version"
407  "Service: Incorrect master key version"
408  "Bad byte order"
409  "Principal unknown"
410  "Principal not unique"
411  "Principal has null key"
412  "Permission Denied"
413  "Can't read ticket file"
414  "Can't find ticket"
415  "Ticket granting ticket expired"
416  "Can't decode authenticator"
417  "Ticket expired"
418  "Ticket issue date too far in the future"
419  "Repeat request"
420  "Ticket for wrong server"
421  "Request inconsistent"
422  "Time is out of bounds"
423  "Incorrect network address"
424  "Protocol version mismatch"
425  "Illegal message type"
426  "Message integrity error"
427  "Message duplicate or out of order"
428  "Unauthorized request"
429  "Current password is NULL"
430  "Current password incorrect"
431  "Protocol error"
432  "Error returned by KDC"
433  "Null ticket returned by KDC"
434  "Retry count exceeded"
435  "Can't send request"
436  "Warning: Not ALL tickets returned"
437  "Generic error"
438  "Don't have ticket granting ticket"
439  "No ticket file"
440  "Can't access ticket file"
441  "Can't lock ticket file; try later"
442  "Bad ticket file format"
443  "Read ticket file before tf_init"
444  "Bad Kerberos name format"
445  "Generic kerberos error"
446  "Authentication"
447  "Intruder alert"
448  "Non-IP address families not supported, sorry"
449  "Authentication is of wrong type"
450  "Encryption key is weak"
451  "Encryption Key parity error"
452  "Location Broker Cache Manager"
453  "Location entry not found"
454  "Entry handle is no longer valid"
455  "No more room in cache for entry"
456  "No servers found"
457  "Latency period has not expired"
458  "No socket families found"
459  "Can't open lbcm cache"
460  "Can't map lbcm cache"
461  "Can't allocate memory for cache"
462  "Cache ID doesn't match existing cache"
463  "data access manager"
464  "invalid key count"
465  "invalid record length"
466  "invalid key length"
467  "memory allocation failed"
468  "record not found"
469  "invalid key number"
470  "no more entries"
471  "record already exists"
472  "handle is read-only"
473  "file I/O error"
474  "data store manager"
475  "could not create database file"
476  "could not open database file"
477  "incompatible format version"
478  "attempt to write same block twice"
479  "specified header size too large"
480  "invalid pointer"
481  "info text too long"
482  "file is already in use"
483  "invalid marker"
484  "attribute broker"
485  "All entries have been returned"
486  "Attempt to set attr. to inconsistent value"
487  "Can't read entry in known attribute table"
488  "Can't add entry to the known attribute table"
489  "Can't read entry in the known object table"
490  "Can't add entry to the known object table"
491  "Can't store attr value record"
492  "Can't retrieve attr value record"
493  "Invalid lookup marker parameter"
494  "Invalid expression tree parameter"
495  "Stale marker (i.e. has expired)"
496  "Can't add entry to the object list table"
497  "Can't create an object list"
498  "Can't delete entry from the object list table"
499  "Can't delete an object list"
500  "Invalid attribute expression"
501  "Can't add entry to an object list"
502  "Can't delete entry from an object list"
503  "Can't read entry in an object list"
504  "attribute broker client agent"
505  "Bad token in attribute expression"
506  "Malformed attribute expression"
507  "Can't convert attribute name to UUID"
508  "network license server"
509  "server"
510  "License not found in database"
511  "iFOR/LS license not found in database"
512  "Past expiration date on license"
513  "Past expiration date on iFOR/LS license"
514  "Before start date on license"
515  "Before start date on iFOR/LS license"
516  "Version not found in database"
517  "iFOR/LS license version not found in database"
518  "Not enough licenses"
519  "Not enough iFOR/LS licenses"
520  "Not authorized to use product"
521  "Wait entry deleted"
522  "Bad I/O"
523  "Other users are waiting"
524  "No licenses available"
525  "No iFOR/LS licenses available"
526  "Still waiting for a license"
527  "No active license to be released"
528  "License is no longer granted to this user"
529  "Time disparity too large"
530  "Vendor id not in database"
531  "Duplicate vendor id"
532  "Duplicate product record for this vendor"
533  "No such product for this vendor"
534  "Could not delete vendor - still has products"
535  "Database not valid for this server"
536  "No such vendor"
537  "Database corrupt"
538  "Warning: product name changed"
539  "Vendor \"Apollo\" can not be deleted"
540  "Fatal error - check error log"
541  "Vendor key is incorrect"
542  "iFOR/LS server not found"
543  "Wrong version of iFOR/LS Server"
544  "iFOR/LS log file contains invalid data"
545  "Start date of license is before compound derived start date"
546  "Expiration date of license is after compound derived expiration date"
547  "License type contradicts license type specified in compound password"
548  "Aggregate duration is less than duration specified"
549  "License has not expired - cannot delete"
550  "License annotation field is incorrect"
551  "Attempting to install an existing product under a different name"
552  "library"
553  "iforlslib not initialized"
554  "Not waiting in any queue"
555  "Not bound to any server"
556  "No matching socket families found"
557  "No servers available for this vendor"
558  "Vendor id is invalid"
559  "Bad parameter"
560  "Job id is invalid"
561  "Cannot create nodelock file"
562  "No valid server handle exists, cannot log message"
563  "tools"
564  "Product version incorrect"
565  "Password contains invalid characters"
566  "Incorrect target in password"
567  "Invalid password or vendor incorrect"
568  "Internal error"
569  "1.0 or 1.1 license not bundled"
570  "Nodelocked license cannot be added to server database"
571  "usage metering server"
572  "Vendor's usage meter license not found"
573  "No usage meter license for this product"
574  "No usage meter license for this version"
575  "Usage meter record not found"
576  "Vendor is not owner of this usage meter record"
577  "Buffer too small"
578  "All usage meter record(s) to be purged not marked as deleted"
579  "Some usage meter record(s) to be purged not marked as deleted"
580  "Usage meter record marked as deleted"
581  "Function not implemented for this operating system"
582  "Status record not found"
583  "usage metering library"
584  "umlib not initialized"
585  "unused"
586  "Vendor is incorrect"
587  "Requested server not found"
588  "Usage meter database modified since last read"
$
$ ------------------------------ Global Location Broker -----------------------
$
$ DEVELOP
589  "GLB: "
$
$ NOTRANS
590  "r"
$
$ The next 4 messages are DEVELOP.
$
591  "Can't open '%%1$s', errno=%%2$d\n"
592  "(find_server_nbrd) Error reading '%%1$s', errno=%%2$d\n"
593  "(find_server_nbrd) Error reading '%%1$s' - file is empty\n"
594  "(find_server_nbrd) no more potential sites\n"
$
$ The next 4 messages are NOTRANS.
$
595  "\n"
596  "\0"
597  "#"
598  " "
$
$ The next 7 messages are DEVELOP.
$
599  "(find_server_nbrd) trying site '%%1$s'\n"
600  "(find_server_nbrd) error converting to sockaddr (0x%%1$lx)\n"
601  "(find_server_nbrd) family mismatch '%%1$s' (%%2$d) and %%3$d\n"
602  "(find_server_nbrd) '%%1$s' LLB lookup failure (0x%%2$lx)\n"
603  "(find_server_nbrd) '%%1$s' LLB doesn't have a GLB registered (0x%%2$lx)\n"
604  "(find_server_nbrd) LLB lookup result %%1$d, invalid family (%%2$d)\n"
605  "(find_server_nbrd) LLB doesn't have GLB registered with valid family\n"
$
$ ADMIN message.
606  "'couldn't convert name'"
$
$ The next 12 messages are DEVELOP.
$
$ %1 is the socket and %2 is the port
607  "(find_server_nbrd) GLB registered at '%%1$s[%%2$ld]'\n"
608  "(find_server_nbrd) couldn't set glb binding (0x%%1$lx)\n"
609  "(find_server_nbrd) RPC runtime failure on glb lookup(0x%%1$lx)\n"
610  "dump_xentry:\n"
611  "    object:   %%1$s\n"
612  "    obj_type: %%1$s\n"
613  "    obj_intf: %%1$s\n"
614  "    flags:    0x%%1$x\n"
615  "    annot:    %%1$s\n"
616  "    slen:     %%1$d\n"
617  "couldn't convert saddr"
$
$ %1 is the socket and %2 is the port
618  "    saddr:    %%1$s[%%2$d]\n"
$
$ The next 7 messages are ADMIN.
$
619  "GLBD: "
620  "<unknown>"
$
$ %1 is the length of %2 which is the type of socket that was to be listened on
621  "GLBD: unable to listen on %%1$.*2$s address family\n"
622  "GLBD: unable to listen on any address families\n"
623  "non-replicated GLB"
624  "non-replicated glb"
625  "nrglbd/nck version 1.5.1"
$
$ NOTRANS.
626  "glbd"
$
$ ADMIN.
627  "Listening...\n"
$
$ ---------------------------- Location Broker Administrator ------------------
$
$ The next 14 messages are ADMIN.
$
628  "lb_admin/nck version 1.5.1"
629  "add"
630  "clean"
631  "delete"
632  "register"
633  "unregister"
634  "set_broker"
635  "set_timeout"
636  "use_broker"
637  "lookup"
638  "exit"
639  "quit"
640  "help"
641  "?"
$
$ NOTRANS.
642  ""
$
$ The next 2 messages are ADMIN.
643  "garbage_collect"
644  "gc"
$
$ The next 2 messages are NOTRANS.
645  "*"
646  "/0"
$
$ The next 2 messages are ADMIN.
$
$ %2 is the name of the socket and %1 is the size of that name
647  "Entries @ global broker: %%2$.*1$s"
$
$ %2 is the name of the socket and %1 is the size of that name
648  "Entries @ local broker: %%2$.*1$s"
$
$ The next 3 messages are NOTRANS.
649  "-nq"
650  "lb_admin: "
651  "\0"
$
$ ADMIN message.
652  "Known commands are:"
$
$ The next 5 messages are NOTRANS.
653  "\n\t"
654  "["
655  "]"
656  "%%1$-15.15s "
657  "\n"
$
$ The next 4 messages are ADMIN.
658  "help                            -- yields list of known commands\nhelp command1 { command2 ... }  -- yields information on specified commands\n"
659  "exit or quit                    -- terminate session\n"
660  "bye.\n"
661  "Unknown command: %%1$s\n"
$
$ NOTRANS
662  "?(lb_admin) "
$
$ The next 20 messages are ADMIN.
$
663  "global"
664  "local"
665  "\tUnknown option '%%1$s'\n"
666  "unregister object type interface location\n                           -- unregisters the specified entry; '*' is a wildcard\n"
667  "Usage: unregister OBJ TYPE INTERFACE LOCATION\n"
668  "register object type interface location annotation { global | local }\n                              -- registers the specified entry\n"
669  "Usage: register OBJ TYPE INTERFACE LOCATION ANNOTATION {global | local }\n"
670  "set_timeout { short | long }    -- set rpc timeout mode\n"
671  "Using %%1$s RPC timeouts\n"
672  "short"
673  "long"
674  "clean obj type interface location -- cleanup registry database\n"
675  "set_broker { local | global } location\n                                -- select broker location\n"
676  "Usage: set_broker { local | global } LOCATION\n"
677  "%%1$s - illegal broker specification\n"
678  "use_broker local | global       -- select local or global broker for operations\n"
679  "Using global broker @"
680  "Using local broker @ "
681  "\nlookup object type interface location  -- search for matching entries\n"
682  "<unknown host> "
$
$ %1 is the length of %2 which is the socket name, %3 is the port.  NOTRANS.
683  "%%2$.*1$s[%%3$lu] "
$
$ ADMIN message.
684  "<invalid address> "
$
$ The next 8 messages are NOTRANS.
$
685  "^"
686  "A"
687  "\\0%%1$03.3o"
688  "\"%%1$s\""
689  " @ "
690  "%%1$s\n"
691  "------------\n"
692  "%%1$12s = *\n"
$
$ ADMIN message.
693  "object"
$
$ %1 is the service, %2 is the name that servive is presently called. NOTRANS.
694  "%%1$12s = %%2$s\n"
$
$ The next 2 messages are ADMIN.
695  "type"
696  "interface"
$
$ NOTRANS.
697  " "
$
$ The next 5 messages are ADMIN.
$
698  "go"
699  "yes"
700  "\n        Automatic deletion in progress - this entry retained\n"
701  "\n        Invalid Address Family.  Delete? "
702  "working "
$
$ NOTRANS.
703  ". "
$
$ The next 20 messages are ADMIN.
$
704  "\n        Server not responding - unable to contact remote site."
705  "\n        Server not responding, but registered in remote llbd database."
706  "\n        Unable to communicate over selected address family."
707  "\n        Therefore unable to verify server status."
708  "\n        Server not responding, and is NOT registered with remote llbd.  %%1$sDelete? "
709  "\n        [using short timeouts] "
710  "\n        Server not responding, and is NOT registered with remote llbd."
711  "\n        Automatic Deletion - entry DELETED.\n"
712  "\n        Server does not support this interface.  %%1$sDelete? "
713  "\n        Server does not support this interface."
714  "  %%1$sDelete? "
715  "No entries match.\n"
716  "delete ? "
717  "\n%%1$ld entries deleted of %%2$ld entries processed\n"
718  "\n    Data from GLB replica: %%1$s\n\n"
719  "STATUS failed %%1$d\n"
720  "use_broker local\n"
721  "use_broker global\n"
722  "set_broker local %%1$s\n"
723  "set_broker global %%1$s\n"
$
$ NOTRANS
724  "%%1$s"
$
$ ADMIN message.
725  "    lookup %%1$s\n"
$
$ NOTRANS
726  "                     %%1$s\n"
$ 
$ The next 2 messages are ADMIN.
727  "     clean %%1$s\n"
728  "  register %%1$s\n"
$
$ NOTRANS.
729  "                     \"%%1$s\" @ "
$
$ The next 2 messages are ADMIN.
730  " global"
731  "unregister %%1$s\n"
$
$ The next 4 messages are NOTRANS.
$
732  "                     %%1$s"
733  "\0"
$
$ ----------------------------- Global Location Broker Finder -----------------
$
734  "-"
735  "%%1$s\n"
$
$ The next 7 messages are ADMIN.
$
736  "lb_find/nck version 1.5.1"
737  "lb_find: bind"
738  "lb_find: setsockopt(SO_BROADCAST)"
739  "lb_find: sendto"
740  "sent to broadcast address %%1$s\n"
741  "waiting for replies\n"
742  "lb_find: select"
$
$ NOTRANS.
743  "."
$
$ The next 2 messages are ADMIN.
744  "lb_find: recvfrom"
$
$ %1 is the packet, %2 is the server name, %3 is the internet address
$ %d is the port number
745  "received %%1$s from glb daemon at %%2$s(%%3$s) port %%4$d.\n"
$
$ NOTRANS.
746  "\n"
$
$ DEVELOP.
747  "glb_ca__get_server_address: communication failure\n"
$
$ The next 3 messages are ADMIN.
748  "no glb server found\n"
749  "default"
750  "alternate_%%1$d"
$
$ The next 2 messages are NOTRANS.
751  "/"
752  "glbd"
$
$ The next 4 messages are ADMIN.
$
753  "replicatable"
754  "non-replicated "
755  "non-replicatable"
756  "unknown"
$
$ %1 is the daemon, %2 is the server name, %3 is the cell name, %4 is the uuid
$ NOTRANS
757  "%%1$s   %%2$s   %%3$s     %%4$s\n"
$
$ ------------------------------- Local Location Broker -----------------------
$
$ ADMIN message.
758  "llbd/nck version 1.5.1"
$
$ NOTRANS
759  "\0"
$
$ DEVELOP.
760  "LLBD: (find_socket) called with no ready sockets\n"
$
$ ADMIN message.
761  "LLBD: "
$
$ The next 2 messages are DEVELOP.
762  "(process_requests) input from"
763  "(process_requests) forwarding to"
$
$ ADMIN message.
764  "< name not available >"
$
$ %1 is the desired message, %3 is the name of the socket, 
$ %2 is the size of that socket, %4 is the port. NOTRANS.
765  "%%1$s %%3$.*2$s[%%4$d]\n"
$
$ The next 3 messages are DEVELOP.
$
766  "LLBD: (use_family) Can't create socket\n"
767  "LLBD: (use_family) Can't bind socket\n"
768  "Listening on:"
$
$ ADMIN message.
769  "LLBD: Too many families specified, %%1$s ignored\n"
$
$ DEVELOP.
770  "LLBD: unable to obtain %%1$.*s family port\n"
$
$ NOTRANS.
771  "llbd"
$
$ ADMIN message.
772  "-listen"
$
$ NOTRANS.
773  "-"
$
$ The next 2 messages are ADMIN.
774  "LLBD: Unable to use family %%1$s\n"
775  "LLBD: Unable to obtain any sockets\n"
$
$ The next 2 messages are NOTRANS.
776  "%%1$s"
$
$ %1 is the file and %2 is the error message
777  "%%1$s: %%2$s"
$
$ ---------------------------------- MS/DOS specific --------------------------
$
$
$ The next 8 messages are NOTRANS.
$
778  "recvfrom: packet ready\n"
779  "sending datagram on socket[%%1$d]:\n"
780  "Error (sendto_nck): cmd failed, retval=0x%%1$x (%%2$s)\n"
781  "getting ready to receive on socket[%%1$d]:\n"
782  "Error (recv_start_rbuf): failed, retval=0x%%1$x (%%2$s)\n"
783  "cancelling receive on socket[%%1$d] rbuf[%%2$d]:\n"
784  "Error (recv_cancel_rbuf): cmd failed, retval=0x%%1$x (%%2$s)\n"
785  "Error (recv_cancel_rbuf): cmd failed, retval=0x%%1$x\n"
$
$ The next 31 messages are DEVELOP.
$
786  "(restart_recv): failed to restart rbuf%%1$d\n"
787  "socket__dump: socket %%1$d out of bounds\n"
788  "in_use: %%1$d\n"
789  "bound: %%1$d\n"
790  "socket__dump: can't convert remote address to string\n"
791  "saddr:%%1$s[%%2$lu]\n"
792  "xmit_cnt: %%1$ld\n"
793  "xmit_failure_cnt: %%1$ld\n"
794  "n_rbufs: %%1$d\n"
795  "rqb_head[%%1$d]: %%2$d\n"
796  "rqb_tail[%%1$d]: %%2$d\n"
797  "rqb_q[%%1$d]: "
798  "%%1$2d "
799  "\n"
800  "rbufs:\n"
801  "ports_in_use: "
802  "0x%%1$04x "
803  "(max=%%1$d)\n"
804  "socket[%%1$d]:\n"
805  "XPORT error counts:\n"
806  "??unknown??"
807  "%%1$2d: %%2$5ld %%3$s\n"
808  "rbuf[%%1$d]:\n"
809  "recv_cnt: %%1$ld\n"
810  "recv_started_cnt: %%1$ld\n"
811  "recv_failure_cnt: %%1$ld\n"
812  "cmd_completed: %%1$d\n"
813  "acb:\n"
814  "sock_no: %%1$d\n"
815  "cmd not completed\n"
816  "rbuf_pool is empty\n"
817  "JUNKSTUFF"
$
$ The next 14 messages are NOTRANS.
$
$ Internet address.  
818  "%%1$d.%%2$d.%%3$d.%%4$d"
819  "%%1$u.%%2$u.%%3$u.%%4$u"
820  "con"
821  "r"
822  "\0"
823  "Warning (alarm): can't set exit() function for alarm\n"
824  "alarm__postponed_cnt = %%1$ld\n"
825  "alarm__not_postponed_cnt = %%1$ld\n"
826  "alarm__reenter_cnt = %%1$ld\n"
827  "(alarm__enable): count < 0\n"
828  " "
$
$ ----------------------------------- RPC modules -----------------------------
$
829  "NA"
830  "ERROR"
$
$ %1 is the size of %2 which is the socket name. %3 is the port
831  "%%2$*.1$s[%%3$lu]"
$
$ ADMIN message.
832  "Ran out of sockets"
$
$ The next 7 message are DEVELOP.
$
833  "(alloc_socket) Can't create socket, errno=%%1$d\n"
834  "(alloc_socket) Can't bind socket, errno=%%1$d\n"
835  "(alloc_socket) Can't set socket %%1$d to non_blocking\n"
836  "(send_pkt) cannot enable broadcast, status=%%1$08lx\n"
837  "(send_pkt) cannot enable broadcast, errno=%%1$d\n"
838  "(c_recv_pkt) Rcvd callback (ptype=%%1$s)\n"
839  "(c_recv_pkt) Rcvd packet not for my activity\n"
$
$ ADMIN message.
840  "Can't register call"
$
$ The next 2 messages are DEVELOP.
841  "(ack_replies) Acking [%%1$s, %%2$lu]\n"
842  "(ack_replies) Can't send ACK, errno=%%1$d\n"
$
$ ADMIN message.
843  "RPC reply acknowledger"
$
$ The next 13 messages are DEVELOP.
$
844  "(await_reply) timeout\n"
845  "(await_reply) select failed, errno=%%1$d\n"
846  "(await_reply) timeout (EINTR)\n"
847  "(await_reply) select returned junk, n_found=%%1$#x, errno=%%2$d\n"
848  "(rpc__sar) can't get encr, status=%%1$08lx\n"
849  "(rpc__sar) broadcasting on reliable transport -- ignored\n"
850  "(rpc__sar) send_pkt failed on frag %%1$d\n"
851  "(rpc__sar) Resending (wait/nocall); seq=%%1$lu, frag=%%2$u\n"
852  "(rpc__sar) Anomalous response to request (ptype=%%1$s)\n"
853  "(rpc__sar) Resending broadcast (wait/timeout)\n"
854  "(rpc__sar) Reduced blast size to %%1$u (timeout), frag=%%2$u\n"
855  "(rpc__sar) Resending (wait/timeout); seq=%%1$lu, frag=%%2$u\n"
856  "(rpc__sar) Starting to ping; seq=%%1$lu\n"
$
$ ADMIN.
857  "select failed"
$
$ The next 22 messages are DEVELOP.
$
858  "(rpc__sar) Too many pings...signaling\n"
859  "(rpc__sar) PING send_pkt failed\n"
860  "(rpc__sar) Reduced blast size to %%1$u (ping/nocall), frag=%%2$u\n"
861  "(rpc__sar) Resending (ping/nocall); seq=%%1$lu, frag=%%2$u\n"
862  "(rpc__sar) Anomalous response to ping (ptype=%%1$s)\n"
863  "(rpc__sar) Rcvd \"working\" during frag send!\n"
864  "(rpc__sar) Rcvd \"working\" pkt; seq=%%1$lu\n"
865  "(rpc__sar) Got \"fack\" for non-large request!\n"
866  "(rpc__sar) Got non-frag \"response\" during frag recv!\n"
867  "(rpc__sar) sending premature FACK, frag=%%1$u unacked_dups=%%2$u acked_dups=%%3$u\n"
868  "(use_family) Can't create socket\n"
869  "(use_family) Can't bind socket, errno=%%1$d\n"
870  "(rpc__use_family) Can't getsockname, errno=%%1$d\n"
871  "(rpc__use_family) Can't get my netaddr\n"
872  "(execute_call_loop) Spurious quit_activity fault ignored\n"
873  "(execute_call_loop) Exiting, st=%%1$08lx\n"
874  "(execute_call_loop) ec2__wait error, st=%%1$08lx, ecvs=(%%2$lu, %%3$lu), ahint=%%4$d\n"
875  "(check_for_pkt) Rcvd (ptype=%%1$s)\n"
876  "(check_for_pkt) dropping request packet in interrupt\n"
877  "(rlisten) Received check-sockets fault\n"
878  "(rlisten) Exiting, st=%%1$08lx\n"
879  "(rlisten) select failed: %%1$d, errno=%%2$d\n"
$
$ The next 3 messages are ADMIN.
880  "RPC packet checker"
881  "RPC listener"
882  "Call executer %%1$u"
$
$ The next 14 messages are DEVELOP.
883  "frag"
884  "in_who_are_you"
885  "working"
886  "in_reply"
887  "replied"
888  "idle"
889  "passive"
890  "(get_activity) Can't allocate slot\n"
891  "(scan_activities) being postponed\n"
892  "(scan_activities) Freeing passive [%%1$s]\n"
893  "(scan_activities) Passivating idle [%%1$s]\n"
894  "(scan_activities) Dropping reply [%%1$s]\n"
895  "(scan_activities) Retransmitting reply (state=%%1$s) [%%2$s]\n"
896  "(scan_activities) Retrying queued request [%%1$s]\n"
$
$ ADMIN message.
897  "RPC activity scanner"
$
$ The next 4 messages are DEVELOP.
$
898  "(ping_common) Working (ptype=%%1$s) [%%2$s]\n"
899  "(ping_common) Resending reply (state=%%1$s, ptype=%%2$s, frag=%%3$u) [%%4$s]\n"
900  "(ping_common) Reduced blast size to %%1$u, frag=%%2$u\n"
901  "(ping_common) No call (state=%%1$s, ptype=%%2$s) [%%3$s]\n"
$
$ The next 2 messages are ADMIN.
902  "(ping_common) Invalid activity state (state=%%1$s, ptype=%%2$s)\n"
903  "invalid activity state"
$
$ The next 10 messages are DEVELOP.
904  "(who_are_you) Doing who-are-you callback\n"
905  "(who_are_you) fault, st=%%1$08lx\n"
906  "(who_are_you) who_are_you failed, st=%%1$08lx\n"
907  "(handle_request_frag) sending premature FACK, frag=%%1$u unacked_dups=%%2$u acked_dups=%%3$u\n"
908  "(quit_activity) quitting %%1$08lx\n"
909  "(rpc__execute_call) Fault while executing request, st=%%1$08lx\n"
910  "(setup_call) queued request [%%1$s]\n"
911  "(do_request) Unknown interface [%%1$s]\n"
912  "(do_request) Opnum out of range [%%1$s]\n"
913  "(do_request) Got inappropriate request (state=%%1$s) [%%2$s]\n"
$
$ %1 is the state, %2 is the packet's UID string. ADMIN message.
914  "(do_request) Invalid state (state=%%1$s) [%%2$s]"
$
$ The next 15 messages are DEVELOP.
915  "(do_request) couldn't get encr (status=%%1$08lx)\n"
916  "(do_request) couldn't decrypt pkt (status=%%1$08lx)\n"
917  "(do_request) Old sequence, previous=%%1$ld [%%2$s]\n"
918  "(do_request) Protocol error [%%1$s]\n"
919  "(do_ping) No call (no activity for this call) [%%1$s]\n"
920  "(do_ping) No call (higher numbered ping), ahint=%%1$d, previous=%%2$ld [%%3$s]\n"
921  "(do_ping) Drop ping [%%1$s]\n"
922  "(do_ack) No or incorrect ahint in ack, ahint=%%1$d [%%2$s]\n"
923  "(do_fack) No or incorrect ahint in fack, ahint=%%1$d [%%2$s]\n"
924  "(do_fack) Anomalous fack, state=%%1$s, prev_seq=%%2$lu [%%3$s]\n"
925  "(do_quit) No call (no activity for this call) [%%1$s]\n"
926  "(do_bad_pkt) Bad pkt, ptype=%%1$s [%%2$s]\n"
927  "(rpc__int_listen_dispatch) Trash ahint (%%1$u)\n"
928  "(rpc__int_listen_dispatch) Trash ihint (%%1$u)\n"
929  "(rpc__int_listen_dispatch) Server boot time mismatch\n"
$
$ The next 3 messages are NOTRANS.
930  "[%%1$08x] %%2$s"
931  "]"
932  "/"
$
$ %1 is the error message, %2 is the file and %3 is the line number. ADMIN.
933  "(rpc) *** FATAL ERROR \"%%1$s\" at %%2$s\\%%3$d ***\n"
$
$ the next 13 statements are DEVELOP messages.
$
934  "request"
935  "ping"
936  "response"
937  "fault"
938  "working"
939  "nocall"
940  "reject"
941  "ack"
942  "quit"
943  "fack"
944  "quack"
945  "BOGUS PACKET TYPE"
946  "< name not available >"
$
$ %1 is socket information. %2 is the size of %3 which is the socket name,
$ %4 is the port.  NOTRANS.
947  "%%1$s %%3$.*2$s[%%4$d]\n"
$
$ Packet uuid, sequence, fragment. NOTRANS.
948  "%%1$s, %%2$lu.%%3$u"
$
$ ADMIN.
949  "(rpc__lock) Can't get %%1$d lock at %%2$s\\%%3$d\n"
$
$ The next 3 are NOTRANS.
950  "(rpc__recvfrom) recvfrom failed s=%%1$d"
951  "(rpc__recvfrom) s=%%1$d received %%2$d bytes "
952  "from"
$
$ The next 3 are DEVELOP.
953  "(rpc__unpack_hdr) Bad RPC version (%%1$u)\n"
954  "(rpc__unpack_hdr) Bad pkt type (%%1$d)\n"
955  "(rpc__unpack_hdr) Packet too short; is %%1$u, data len is %%2$u\n"
$
$ The next 11 messages are NOTRANS.
956  "(rpc__sendto) Drop pkt [%%1$s]\n"
957  "(rpc__sendto) Stash pkt [%%1$s]\n"
958  "(rpc__sendto) Re-xmit pkt [%%1$s]\n"
959  "(rpc__sendto) s=%%1$d failed %%2$d bytes "
960  "to"
961  "(rpc__sendto) sendto failed"
962  "(rpc__sendto) sendto returned 0\n"
963  "(rpc__sendto) s=%%1$d sent %%2$d bytes "
964  "(rpc__malloc) called while executing call\n"
965  "(rpc__free) called while executing call\n"
966  "(rpc__process_fack) rcvd=%%1$3lu, curr=%%2$3lu, bad=%%3$d\n"
$
$ DEVELOP.
967  "(rpc__process_fack) Pkts/blast reduced to %%1$u, frag=%%2$u\n"
$
$ The next 4 messages are ADMIN.
968  "(periodic_task) \"%%1$s\" task exited, status=%%2$08lx\n"
969  "Can't create periodic task"
970  "Can't create task"
$
$ %1 is a message, %2 is the status code
971  "%%1$s status %%2$08lx\n"
$
$ %1 is the message, %2 is the status code plus possibly more information
$ NOTRANS.
972  "%%1$s%%2$s\n"
$
$ The next 3 are DEVELOP.
973  "\t# of remote calls out:\t%%1$lu\n"
974  "\t# of remote calls in:\t%%1$lu\n"
975  "\t# of pkts sent:\t%%1$lu\n"
$
$ %1 is the packet name, %2 is the number of packets . NOTRANS.
976  "\t\t%%1$s\t%%2$lu\n"
$
$ The next 20 are DEVELOP.
$
977  "\t# of pkts rcvd:\t%%1$lu\n"
978  "\t# of frag resends:\t%%1$lu\n"
979  "\t# of dup frags rcvd:\t%%1$lu\n"
980  "rpc_vers: %%1$d\n"
$
$ %1 is the packet type, %2 is the packet name
981  "xptype: %%1$d (%%2$s)\n"
982  "flags: 0x%%1$x\n"
983  "int_rep: %%1$d\n"
984  "char_rep: %%1$d\n"
985  "float_rep: %%1$d\n"
986  "object uuid: %%1$s\n"
987  "interface uuid: %%1$s\n"
988  "activity uuid: %%1$s\n"
989  "server boot time: %%1$ld\n"
990  "interface version: %%1$ld\n"
991  "sequence number: %%1$ld\n"
992  "operation number: %%1$d\n"
993  "interface hint: %%1$d\n"
994  "activity hint: %%1$d\n"
995  "fragment #: %%1$d\n"
996  "length of body: %%1$d\n"
$
$ The next 4 messages are NOTRANS.
997  "\n"
998  "%%1$04x: "
999  " "
1000  "%%1$02x "
$
$ The next 3 messages are DEVELOP.
$
1001  "ver ptyp f1 f2    seq/fnum  ihnt ahnt  len              interface/ver/op                            activity                  sboot                    object                drep   at\n"
1002  "--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------\n"
$
$ the format of a dumped packet
1003  "%%1$c%%2$c%%3$1u %%4$4.4s %%5$02x %%6$02x %%7$6lu/%%8$05u %%9$04x %%1004x %%114d %%12s/%%1302u/%%1403u %%15s %%169u %%17s %%1802x%%1902x%%2002x%%2102x %%2202x\n"
$
$ The next 2 are NOTRANS.
1004  "R"
1005  "*"
$
$ The next 3 are USER.
$
1006  "status %%1$08lx"
$
$ %1 is the status code, 
$ %3 is the subsystem name and %2 is the length of that name
1007  "status %%1$08lx (%%3$.*2$s)"
$
$ %1 is the status code, 
$ %3 is the subsystem name and %2 is the length of that name
$ %5 is the module name and %4 is the length of that name
1008  "status %%1$08lx (%%3$.*2$s/%%5$.*4$s)"
$
$ %2 is the error text and %1 is the length of that error
$ %4 is the subsystem name and %3 is the length of that name
$ %6 is the module name and %5 is the length of that name. NOTRANS.
1009  "%%2$.*1$s (%%4$.*3$s/%%6$.*5$s)"
$
$ DEVELOP
1010  "StopWatch splits:  (overhead: %%1$.3f msecs)\n"
$
$ %1 is a file name, %2 is a line number. NOTRANS.
1011  "(%%1$s:%%2$d)"
$
$ %1 is the iteration, %2 is this iterations id, 
$ %3 is the previous message, %4 and %5 are timing statistics. NOTRANS.
1012  "  %%1$2d. %%2$-30s %%3$-20s  %%4$7.3f msecs  dt=%%5$.3f\n"
$
$ The next 10 messages are ADMIN.
1013  "user"
1014  "none"
1015  "nobody"
1016  "other"
1017  "1st fork failed"
1018  "Can't setgid to %%1$d (errno=%%2$d)\n"
1019  "Can't setgroups to empty list (errno=%%1$d)\n"
1020  "Can't setuid to %%1$d (errno=%%2$d)\n"
1021  "2nd fork failed"
1022  "execv of \"%%1$s\" failed (errno=%%2$d)\n"
$
$ ------------------------------ Socket Interface Modules ---------------------
$
$ The first 4 messages are NOTRANS.
$
1023  "dds"
1024  "ip"
1025  "ns"
1026  "dn"
$
$ ADMIN.
1027  "unspec"
$ 
$ The next 28 messages are NOTRANS.
$
1028  "\0"
1029  ":"
1030  "["
1031  "]"
1032  "#"
$
$ %1 is the type of socket, %2 is the address 
1033  "%%1$s:#%%2$s"
$
$ %1 is the type of socket, %2 is the address
1034  "%%1$s:%%2$s"
$
$ %1 is the network name, %2 is the node name
1035  "%%1$lx.%%2$lx"
1036  "r"
1037  "/"
1038  "nnnnn.nnnnn"
1039  "//"
1040  "#"
1041  "\0"
$
$ %1 is the node and %2 is the hostname
1042  "%%1$lx %%2$s"
$
1043  "\0"
1044  "#"
1045  "%%1$d"
1046  "#%%1$d"
$
$ %1 is the net number and %2 is the node number
1047  "%%1$ld.%%2$ld"
1048  "nnnnn.nnnnn"
$
$ %1 is the net number and %2 is the node number
1049  "%%1$lx.%%2$lx"
1050  "GETHOSTBYADDR                   "
1051  "GETHOSTBYNAME                   "
1052  "GETHOSTNAME                     "
1053  "INET_NTOA                       "
1054  "?.?.?.?"
1055  "INET_ADDR                       "
$
$ The next 2 messages are ADMIN.
1056  "(socket_inet/get interface broadcast address): "
1057  "(socket_inet/get interface address): "
$
$ The next 2 are NOTRANS.
1058  "con"
1059  "r"
$
$ USER.
1060  "(stcode) usage: stcode <hex status code>\n"
$
$ NOTRANS.
1061  "%%1$lx"
$
$ USER.
1062  "(stcode) \"%%1$s\" does not appear to be a valid hex status code\n"
$
$ The next 2 are NOTRANS.
1063  "%%1$s\n"
1064  "ridge"
$
$ The next 2 messages are USER.
1065  "*** Exiting: exception caught by PFM system cleanup handler\n*** status %%1$08lx\n"
1066  "*** Exiting: exception caught by PFM system cleanup handler\n*** %%1$s\n"
$
$ -------------------------------- UUID Interface Modules ---------------------
$
$ The next 5 messages are NOTRANS.
$
1067  "\0"
1068  "r"
1069  "#include"
1070  "#"
1071  "%%1$s/uuidname.txt"
$
$ ADMIN.
1072  "uuid_gen/nck version 1.5.1"
$
$ NOTRANS.
1073  "\0"
$
$ ADMIN.
1074  "-version"
$
$ The next 2 are NOTRANS.
1075  "%%1$s\n"
1076  "-p"
$
$ ADMIN.
1077  "%%%%pascal\n[\nuuid(%%1$s),\nversion(1)\n]\ninterface INTERFACENAME;\n\nend;\n"
$
$ NOTRANS.
1078  "-c"
$
$ ADMIN.
1079  "%%%%c\n[\nuuid(%%1$s),\nversion(1)\n]\ninterface INTERFACENAME {\n\n}\n"
$
$ The next 5 are NOTRANS.
1080  "-t"
1081  "-C"
$
$ A UUID string expanded
1082  "\n= { 0x%%1$8.8x,\n\\n    0x%%2$4.4x,\n\\n    0x%%3$4.4x,\n\\n    0x%%4$2.2x,\n\\n    {0x%%5$2.2x, 0x%%6$2.2x, 0x%%7$2.2x, 0x%%8$2.2x, 0x%%9$2.2x, 0x%%102.2x, 0x%%112.2x} };\n"
1083  "-P"
$
$ The next 2 messages are ADMIN.
$
$ A UUID string expanded. 
1084  "\n:= [\n    time_high := 16#%%1$8.8x,\n\\n    time_low := 16#%%2$4.4x,\n\\n    reserved := 16#%%3$4.4x,\n\\n    family := chr(16#%%4$2.2x),\n\\n    host := [chr(16#%%5$2.2x), chr(16#%%6$2.2x), chr(16#%%7$2.2x), chr(16#%%8$2.2x), chr(16#%%9$2.2x), chr(16#%%102.2x), chr(16#x)]\2.2x)]\n\\n    ]\n"
$
$ ADMIN message.
1085  "usage: uuid_gen {[-p -c -P -C- t] | [-v[ersion]]}\n"
$
$ The next 158 messages are NOTRANS.
$
1086  "0"
1087  "."
$
$ -------------------------------- VMS specific routines ----------------------
$
1088  "%%1$d.%%2$d.%%3$d.%%4$d\0"
1089  "%%1$d.%%2$d\0"
1090  "#%%1$d\0"
1091  "%%1$*c\0"
1092  "recvpoll(%%1$d): len=%%2$d"
1093  "recvpoll(%%1$d): errno=%%2$d"
1094  "recvpoll(%%1$d): notready"
1095  "recvpoll(%%1$d): start failed errno=%%2$d"
1096  "recvpoll(%%1$d): started"
1097  "recv_ast(%%1$d): errno=%%2$d"
1098  "recv_ast(%%1$d): ICMP error=%%2$d"
1099  "recvpoll(%%1$d): restart failed errno=%%2$d"
1100  "recv_ast(%%1$d): restarted after zero read"
1101  "recv_ast(%%1$d): from=%%2$x:%%3$d len=%%4$d"
1102  "sendto(%%1$d): to=%%2$x:%%3$d len=%%4$d"
1103  "sendto(%%1$d): failed errno=%%2$d"
1104  "sendto_loop(%%1$d): to=%%2$x:%%3$d len=%%4$d"
$
$ %1 is the VMS socket fd, %2 is the error code
1105  "sendto_loop(%%1$d): failed errno=%%2$d"
1106  "\0"
1107  "\0"
1108  "\n"
1109  "dnet_chanabort(%%1$d): disconnecting"
1110  "\0"
1111  "%%1$d"
1112  "dnet_bind(%%1$d): name=dn:%%2$d[%%3$s]"
1113  "dnet_confirm(%%1$d): IO$_ACCESS failed status=%%2$d"
1114  "dnet_confirm(%%1$d): connection accepted ok"
1115  "dnet_reject: IO$_ACCESS|IO$M_ABORT"
1116  "#"
1117  "NCS_"
1118  "dnet_listen(%%1$d): IO$_ACPCONTROL failed status=%%2$d"
1119  "dnet_listen(%%1$d): decl server obj succeeded"
1120  "dnet_mbx_ast(%%1$d): failed status=%%2$d"
1121  "dnet_mbx_ast(%%1$d): DECnet shutdown"
1122  "dnet_mbx_ast(%%1$d): misc mbx msg %%2$d ignored"
1123  "dnet_mbx_ast(%%1$d): connection rejected unit %%2$d"
1124  "dnet_mbx_ast(%%1$d): could not find unit %%2$d to abort"
1125  "dnet_connect(%%1$d): sys$assign failed status=%%2$d"
1126  "%%1$d::\"%%2$s=/LLC................\""
1127  "%%1$d::\"TASK=NCS_%%2$s/LLC................\""
1128  "/"
1129  "\0"
1130  "\0"
1131  "dnet_connect(%%1$d): server object not registered"
1132  "dnet_connect(%%1$d): IO$_ACCESS failed status=%%2$d"
1133  "dnet_connect(%%1$d): name=dn:%%2$d[%%3$s] completed ok"
1134  "dnet_getsockname(%%1$d): name=dn:%%2$d[%%3$s]"
1135  "dnet_getpeername(%%1$d): name=dn:%%2$d[%%3$s]"
1136  "dnet_recvpoll(%%1$d): errno=%%2$d"
1137  "dnet_recvpoll(%%1$d): len=%%2$d"
1138  "dnet_recvpoll(%%1$d): notready"
1139  "dnet_net_read(%%1$d): start failed"
1140  "dnet_recv_ast(%%1$d): no memory to post error"
1141  "dnet_recv_ast(%%1$d): status=%%2$d"
1142  "dnet_recv_ast(%%1$d): empty packet"
1143  "dnet_recv_ast(%%1$d): no socket for local object [%%2$s]"
1144  "dnet_recv_ast(%%1$d): no memory to post packet"
1145  "dnet_recv_ast(%%1$d): from=dn:%%2$d[%%3$s] len=%%4$d"
1146  "dnet_sendto(%%1$d): to=NULL, not connected"
1147  "dnet_sendto(%%1$d): to=dn:%%2$d[%%3$s] len=%%4$d"
1148  "dnet_sendto(%%1$d): slave not found"
1149  "dnet_sendto(%%1$d): lost connection status=%%2$d"
1150  "dnet_sendto(%%1$d): failed status=%%2$d"
1151  ":"
1152  "\0"
1153  "dnet_get_conn_info"
1154  "dnet_get_conn_info(%%1$d): remote from %%2$s(%%3$d)[%%4$s]"
1155  "dnet_get_unit(%%1$d): getdviw failed status=%%2$d"
1156  "NCS$%%1$08x_MBX"
1157  "dnet_init_local"
1158  "\0"
1159  "dnet_gethostbyname"
1160  "dnet_gethostbyaddr"
1161  "\0"
1162  "%%1$s\n"
1163  "*A0:"
1164  "bind(%%1$d): status=%%2$d"
1165  "bind(%%1$d): name=%%2$s:%%3$d"
1166  "fns_setsockopt(%%1$d) opt=%%2$d"
1167  "fns_setsockopt(%%1$d) opt=%%2$d val=%%3$d"
1168  "recvpoll(%%1$d): errno=%%2$d"
1169  "recvpoll(%%1$d): len=%%2$d"
1170  "recvpoll(%%1$d): notready"
1171  "fns_net_read(%%1$d): start failed"
1172  "fns_net_read(%%1$d): started"
1173  "fns_recv_ast(%%1$d): connection rejected"
1174  "fns_recv_ast(%%1$d): cancel"
1175  "fns_recv_ast(%%1$d): errno=%%2$d"
1176  "fns_recv_ast(%%1$d): from=%%2$s:%%3$d len=%%4$d"
1177  "sendto(%%1$d): to=%%2$s:%%3$d len=%%4$d"
1178  "sendto(%%1$d): to=NULL len=%%2$d"
1179  "fns_sendto_loop(%%1$d): to=%%2$x:%%3$d len=%%4$d"
1180  "sendto(%%1$d): restarting errno=%%2$d"
1181  "sendto(%%1$d): failed errno = %%2$d"
1182  "\n"
1183  ":"
1184  "\0"
1185  "FNS_DB"
1186  "r"
1187  "r"
1188  "\n"
1189  "\0"
1190  "twg_bind(%%1$d): status=%%2$d"
1191  "twg_bind(%%1$d): name=%%2$s:%%3$d"
1192  "twg_setsockopt(%%1$d) opt=%%2$d"
1193  "twg_setsockopt(%%1$d) opt=%%2$d val=%%3$d"
1194  "twg_recvpoll(%%1$d): errno=%%2$d"
1195  "twg_recvpoll(%%1$d): len=%%2$d"
1196  "twg_recvpoll(%%1$d): notready"
1197  "twg_net_read(%%1$d): start failed"
1198  "twg_net_read(%%1$d): started"
1199  "twg_recv_ast(%%1$d): connection rejected"
1200  "twg_recv_ast(%%1$d): cancel"
1201  "twg_recv_ast(%%1$d): errno=%%2$d"
1202  "twg_recv_ast(%%1$d): from=%%2$s:%%3$d len=%%4$d"
1203  "twg_sendto(%%1$d): to=%%2$s:%%3$d len=%%4$d"
1204  "twg_sendto(%%1$d): to=NULL len=%%2$d"
1205  "twg_sendto_loop(%%1$d): to=%%2$s:%%3$d len=%%4$d"
1206  "twg_sendto(%%1$d): restarting errno=%%2$d"
1207  "twg_sendto(%%1$d): failed errno = %%2$d"
1208  "\n"
1209  "\0"
1210  "#"
1211  "\0"
1212  "\0"
1213  "\0"
1214  "\0"
1215  "\0"
1216  "r"
1217  "\n"
1218  "\0"
1219  "ucx_bind(%%1$d): status=%%2$d"
1220  "bind(%%1$d): name=%%2$s:%%3$d"
$
$ %1 is the VMS socket fd, %2 is the error code
1221  "ucx_setsockopt(%%1$d) opt=%%2$d"
1222  "ucx_setsockopt(%%1$d) opt=%%2$d val=%%3$d"
1223  "ucx_getsockopt(%%1$d)"
1224  "ucx_getsockopt(%%1$d) opt=%%2$d val=%%3$d"
1225  "recvpoll(%%1$d): errno=%%2$d"
1226  "recvpoll(%%1$d): len=%%2$d"
1227  "recvpoll(%%1$d): notready"
1228  "recvpoll(%%1$d): start failed"
1229  "recvpoll(%%1$d): started"
1230  "recv_ast(%%1$d): connection rejected"
1231  "recv_ast(%%1$d): cancel"
1232  "recv_ast(%%1$d): VAXC error=%%2$x"
1233  "recv_ast(%%1$d): errno=%%2$d"
1234  "recv_ast(%%1$d): from=%%2$s:%%3$d len=%%4$d"
1235  "sendto(%%1$d): to=%%2$s:%%3$d len=%%4$d"
1236  "sendto(%%1$d): to=NULL len=%%2$d"
1237  "sendto(%%1$d): restarting errno=%%2$d"
1238  "sendto(%%1$d): failed errno = %%2$d"
1239  "\0"
1240  "ucx_gethostbyname"
1241  "ucx_gethostbyaddr"
1242  "\n"
1243  "\0"
$
$ The next 40 messages are DEVELOP.
$
1244  "call"
1245  "listen"
1246  "hang_up"
1247  "send"
1248  "receive"
1249  "receive_any"
1250  "send_continue"
1251  "send_datagram"
1252  "receive_datagram"
1253  "send_broadcast"
1254  "receive_broadcast"
1255  "reset_name"
1256  "status"
1257  "cancel"
1258  "illegal data buffer length"
1259  "illegal command"
1260  "command timed out"
1261  "message incomplete"
1262  "VCID out of range"
1263  "no resource available"
1264  "virtual circuit already closed"
1265  "command cancelled"
1266  "local resource error"
1267  "remote resource error"
1268  "virtual circuit ended abnormally"
1269  "interface busy"
1270  "too many commands outstanding"
1271  "command completed while cancel occuring"
1272  "command not valid to cancel"
1273  "invalid net number"
1274  "illegal address"
1275  "command is not yet finished"
1276  "??unknown??"
1277  "ok"
1278  "command: 0x%%1$02x (%%2$s)\n"
1279  "command id number: %%1$u\n"
1280  "virtual circuit id number: %%1$u\n"
1281  "buffer length: %%1$u\n"
1282  "buffer address: 0x%%1$04lx_%%2$04lx\n"
1283  "local network address: "
$ 
$ NOTRANS.
1284  "\n"
$
$ The next 21 messages are DEVELOP.
$
1285  "remote network address: "
1286  "async notification routine: 0x%%1$04lx_%%2$04lx\n"
1287  "local network number: "
1288  "remote network number: "
1289  "receive timeout (500 msecs): %%1$u\n"
1290  "send timeout (500 msecs): %%1$u\n"
1291  "completed command id: %%1$u\n"
1292  "virtual circuit id: %%1$u\n"
1293  "error code: 0x%%1$02x (%%2$s)\n"
1294  "actual buffer length: %%1$u\n"
1295  "host address: "
1296  "net number: "
1297  "hardware status: 0x%%1$02x\n"
1298  "software version: %%1$u\n"
1299  "packets sent: %%1$lu\n"
1300  "packets received: %%1$lu\n"
1301  "packets received short: %%1$u\n"
1302  "packets aborted: %%1$u\n"
1303  "packets misaligned: %%1$u\n"
1304  "packets with crc errors: %%1$u\n"
1305  "address significance: "
$
$ The next 3 messages are NOTRANS.
$
1306  "0x"
1307  "_"
1308  "%%1$02x"
$
$ ADMIN.
1309  "Cannot issue command from this machine."
