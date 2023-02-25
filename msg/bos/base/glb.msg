$ ENGLISH_SOURCE_VERSION= 1.4
$ @(#)25        1.3  src/msg/bos/base/glb.msg, bosmsg, msg411, 9430C411a 7/23/94 12:24:02
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
$ glb.msg
$
$ Message source object file used by the gencat utility for message catalogue
$ generation.  It is a compilation of catalogued messages that were extracted 
$ from the source code modules that comprise the following utilities:
$
$					drm_admin
$					glbd
$
$ -----------------------------------------------------------------------------
$
$quote "
$delset 6
$set 6
$
$ The first 24 messages are NOTRANS.
$
1  "Testing clock_to_sec ... "
2  "Successful!\n"
3  "Test failed.\n"
4  "The standard secs in the clock is %%1$d\n"
5  "My value for the  secs in the clock is %%1$d\n\n"
6  "Testing clock_to_timeval ... "
7  "The timeval is \n\tsecs: %%1$d  usecs %%2$d \n"
8  "MY  timeval time is\n\t secs: %%1$d  usecs %%2$d \n"
9  "The clock time is\n\t high: %%1$d  low %%2$d \n"
10  "Testing timeval_to_clock ... "
11  "MY  clock time is\n\t high: %%1$d  low %%2$d \n"
12  "Testing add_clock ..."
13  "The added clock time from cal is\n\t high: %%1$d  low %%2$d \n"
14  "The added clock time from my add_clock is\n\t high: %%1$d  low %%2$d \n"
15  "The added clock time should simply be 2x the clock.\n"
16  "The test showed the first clock to be %%1$s the second.\n"
17  "equal to"
18  "greater than"
19  "The first clock is in fact smaller than the second.\n"
20  "The first clock time is\n\t high: %%1$d  low %%2$d \n"
21  "The second clock time is\n\t high: %%1$d  low %%2$d \n"
22  "less than"
23  "The first clock is in fact greater than the second.\n"
24  "The first clock is in fact equal to the second.\n"
$
$ ----------------------------- Data Replication Manager ----------------------
$
$ The next 49 messages are ADMIN.
$
25  "creating first replica of object %%1$s.\n"
26  "unable to create object %%1$s.\n"
27  "object %%1$s created.\n"
28  "creating replica of object %%1$s.\n"
29  "unable to create replica of object %%1$s.\n"
30  "replica of object %%1$s created.\n"
31  "opening replica.\n"
32  "unable to open replica of object %%1$s.\n"
33  "replica of object %%1$s opened.\n"
34  "\nUnable to add replica "
35  "\nUnable to delete replica "
$
$ ---------------------- Data Replication Manager Administrator ---------------
$
36  "drm_admin"
37  "drm_admin/nck version 1.5"
38  "set"
39  "info"
40  "stop"
41  "addrep"
42  "reprep"
43  "chrep"
44  "delrep"
45  "purgerep"
46  "lrep"
47  "reset"
48  "merge"
49  "merge_all"
50  "monitor"
51  "quit"
52  "help"
53  "?"
54  "version"
55  ""
56  "set [-obj object_name] -h replica_host_name"
57  "addrep replica_host_name"
58  "reprep has been replaced with a new command:\n \t\tchrep -from replica_host_name1 -to replica_host_name2"
59  "chrep -from replica_host_name1 -to replica_host_name2"
60  "delrep replica_host_name"
61  "purgerep replica_host_name"
62  "lrep [-na -del -clocks]"
63  "reset replica_host_name"
64  "merge -to|-from replica_host_name"
65  "monitor [-repeat period_in_minutes]"
66  "help [command]"
67  "? [command]"
68  "unconfigured"
69  "uninitialized"
70  "in service"
71  "closed"
72  "deleted"
73  "-version"
$
$ The version of the administrator. NOTRANS.
74  "%s\n"
$
$ The next 4 messages are ADMIN.
75  "drm_admin: "
$
$ Command used
76  "Usage: %s\n"
77  "\t**** Process quit received ****\n"
78  "\tCommand aborted - quit received\n"
$
$ The version of the administrator. NOTRANS.
79  "\t%s\n"
$
$ ADMIN message.
80  "Known commands are:"
$
$ The next 3 messages are NOTRANS.
81  "\n\t"
$
$ Command name
82  "%-10.10s "
83  "\n"
$
$ The next 52 messages are ADMIN.
84  "\tUnknown command: %s\n"
85  "-obj"
86  "-host"
87  "-rep"
88  "\tUnknown option %s\n"
89  "\n\tChecking clocks of %s replicas\n"
90  "\tObject not specified\n"
91  "\tBad object name - "
92  "\tBad host name - "
93  "\tCannot locate a %s replica which is in service\n"
94  "\tDefault object: unspecified  "
95  "\tDefault object: %s  "
96  "default host: unspecified  "
97  "default host: %s  "
98  "status: not communicating "
99  "state: %s\n"
100  "\tToo many arguments\n"
101  "\tCannot stop replica - "
102  "\tIncorrect number of arguments - "
103  "\tCannot add replica - "
104  "\tCannot replace replica - "
105  "-to"
106  "-from"
107  "\tBad host name %s - "
108  "\t%s not found on the replica list  "
109  "\tCannot get rpc handle  "
110  "\tCannot change replica. A replica is running at %s\n"
111  "\n\t%1$s and %2$s are different protocol families.\n"
112  "\tIt is not recommended to have hosts from different families on the same replica list.\n"
113  "\n\tDo you really want to change replicas?"
114  "\tThe replica list was NOT changed\n"
115  "\tCannot add replica %s - "
116  "\tBad host address %s - "
117  "\tCannot delete replica %s - "
118  "-force"
119  "\tdelrep -force replica_host_name   has been replaced with a new command:\n"
120  "\t\tpurgerep replica_host_name\n"
121  "\tReplica to be deleted must be specified\n"
122  "\tDefault object not specified - use set command to specify it\n"
123  "\tBad host address - "
124  "purge"
125  "\tReplica %s NOT purged\n"
126  "\tCannot get rpc handle\n"
127  "\tCannot purge replica %s. A replica is running at this host\n"
128  "\tCannot purge replica %s. A replica is running at this host.\n"
129  "\tUse chrep to change its network address on the replica list\n"
130  "\tCannot purge replica %s - "
131  "\tReplica %s NOT reset\n"
132  "\tCannot reset replica %s - "
133  "-na"
134  "-del"
135  "-clocks"
$ 
$ The next 5 messages are NOTRANS.
$
136  "\t(DE) "
137  "\t     "
138  "\t"
$
$ Name of socket
139  "%-22.*s\t"
140  "%-26.*s\t"
$ 
$ The next  3 messages are ADMIN.
141  "\tError reading replica list - "
142  "\t**** Too many replicas - some will not be listed ****\n\n"
143  "                \t*** clock unavailable ***"
$ 
$ %1 is the year, %2 is the month, %3 is the day, %4 is the hour, %5 is the minute
$ NOTRANS.
144  "%1$4d/%2$02d/%3$02d.%4$02d:%5$02d\t"
$
$ The next 4 messages are ADMIN.
$
145  "*** clock skewed ***"
146  "*** clock skew warning ***"
147  "-repeat"
148  "\tMissing argument for -repeat\n"
$
$ The next 3 messages are NOTRANS.
$
149  "\t%-22.*s                 \t*** does not respond ***\n"
150  "\t%-22.*s\t"
151  "\t---\n"
$
$ The next 35 messages are ADMIN.
$
152  "\tMay not merge the default replica from/to itself\n"
153  "\tCannot merge replica - "
154  "merge_all"
155  "\tMerge_all aborted\n"
156  "\tMerge_all successfully completed\n"
157  "\tINCOMPLETE merge_all done\n"
158  "\tDefault replica not communicating or not in service\n"
159  "\tMerge from %.*s done\n"
160  "\tMerge from %.*s aborted - "
161  "\tMerge to %.*s done\n"
162  "\tMerge to %.*s aborted - "
163  "\tThe %s operation is very drastic.  We encourage you to do a merge_all\n"
164  "\t(using an in_service replica as the default replica) before doing the %s.\n"
$
$ %1 is the command to be applied on %2
165  "\n\tDo you want to %1$s %2$s now? (type yes if you do) "
166  "yes"
167  "%s  (type yes if you do) "
168  "\tDefault object or host not specified - use set command to specify them\n"
169  "\tCannot communicate with default replica - "
$
$ ------------------------------- Entry File Management -----------------------
$
170  "Couldn't allocate memory for entry file grow."
171  "Couldn't allocate memory for entry file grow (map)."
172  "Object %%1$s--Bad status on key extract.\n"
173  "Object %%1$s--Duplicate prime key.\n"
174  "Object %%1$s--No match for key on delete lookup.\n"
175  "Object %%1$s--Entry not found in list in delete scan"
176  "Object not found."
177  "Couldn't allocate memory for entry file (map)."
178  "Couldn't allocate memory for entry file."
179  "Error removing entry from in-memory symbol table."
180  "Error adding entry to in-memory symbol table."
181  "Error writing to entry file."
182  "Error writing to propagation file."
183  "Error adding server entry to in-memory symbol table."
184  "Error adding dummy deleted entry to in-memory symbol table."
185  "In-memory symbol table deletion failed."
186  "Write to entry file failed."
$
$ --------------------------- Data Replication Manager Logger -----------------
$
$ The next 3 messages are NOTRANS.
$
187  "\n"
$
$ %1 is the routine name, %2 is the date, %3 is the status
188  "(%%1$s) %%2$s  [%%3$08x] "
$
$ %1 is the name of the routine, %2 is the date
189  "(%%1$s) %%2$s  "
$
$ ADMIN message.
190  "(* name unavailable *)"
$
$ NOTRANS.
191  "\0"
$
$ %1 is the year, %2 is the month, %3 is the day of the month, %4 is the hour, %5 is the minute, %6 is the second
$ ADMIN message.
192  "%%1$04d/%%2$02d/%%3$02d.%%4$02d:%%5$02d:%%6$02d"
$
$ The next 3 messages are NOTRANS.
$
193  "%%1$s  st %%2$lx\n"
194  "Merge to %%1$s\n"
195  "Merge from %%1$s\n"
$
$ ------------------------------ Propagation File Manager ---------------------
$
$ The next 20 messages are ADMIN.
$
196  "Couldn't allocate memory for prop file grow."
197  "Couldn't allocate memory for prop file grow (map)."
198  "Couldn't allocate memory for replica heap."
199  "Object not found."
200  "Pgm specified max replicas less than disk file specified max replicas."
201  "No replica marked as local on replica list."
202  "Couldn't allocate memory for prop file (map)."
203  "Couldn't allocate memory for prop file."
204  "Cannot create new replica list file."
205  "Pgm error - number of prop blocks saved with new file different from number of disk blocks."
$
$ %1 is the name of the routine, %2 is the maximum size of the replica list
206  "(%%1$s) *** Replica list max size increased to %%2$d. ***\n"
207  "Error writing to propagation file."
208  "Propagation file full."
209  "Write to propagation file failed."
210  "\n%%1$d entries on prop queue\n"
211  "No local host found on replica list."
212  "Error writing to replica list file."
$
$ %1 is a replica address, %2 is a replica address, %3 is the reason why this action cannot be done
213  "\nUnable to propagate add replica %%1$s update to replica %%2$s  %%3$s\n"
$
$ %1 is a replica id, %2 is a replica address, %3 is the reason for the problem
214  "\nUnable to propagate delete replica %%1$s update to replica %%2$s  %%3$s\n"
$
$ %1 is a replica address, %2 is the description of the problem
215  "\nUnable to propagate an update to replica %%1$s  %%2$s\n"
$
$ The next 2 messages are NOTRANS.
$
216  "Add replica rqst %%1$s "
217  "Delete replica rqst %%1$s "
$ 
$ ADMIN message.
218  "Propagate: unknown propagation operation."
$
$ The next 4 messages are NOTRANS
$
219  "%%1$s   st %%2$lx\n"
220  "Replica list merged  st %%1$lx\n"
221  "\nProp to %%1$s\n"
222  "*** Drop prop entry  %%1$s\n"
$ 
$ The next 10 messages are ADMIN.
$
223  "DRM propagate/purge task"
224  "Fault in propagate/purge task! (signalling DT and exiting)"
225  "Propagation task quitting, can't get timer EC."
226  "Propagation task quitting, bad ec2__wait status."
227  "Propagation task error... bad database state."
$
$ ------------------------ Data Replication Manager State Handler -------------
$
228  "DRM RPC listener task"
229  "rdrm"
230  "Unable to register with the local location broker."
231  "rdrm_debug"
232  "rdrm_applic"
$
$ The next 2 messages are NOTRANS.
233  "\0"
$
$ The record lengths of these 3 fields
234  "Entry reclen %%1$d  replica reclen %%2$d  prop entry reclen %%3$d\n"
$
$ The next 3 messages are ADMIN.
235  "replica of object %%1$s closed.\n"
236  "replica of object %%1$s deleted.\n"
237  "replica of object %%1$s reset.\n"
$
$ %1 is the error message, %2 is the error code.  NOTRANS.
238  "%%1$s  %%2$lx\n"
$
$ ADMIN
239  "FATAL error  %%1$s\n"
$
$ ------------------------------ Data Storage Management ----------------------
$
$
$ The next 7 messages are DEVELOP.
$
240  "dsm__print_map unavailable.\n"
$
$ %1 is the number of pages, %2 is the size of each individual page  , %3 is the requests not yet fulfilled
241  "DSM map; %%1$d initialized pages of %%2$d bytes; %%3$d allocations pending\n"
$
$ %1 is the number of bytes, %2 is where this chunk is
242  "---- %%1$d byte chunk at %%2$d\n"
$
$ %1 is the first block of a chunk of bytes, %2 is location of preheader, %3 is the page where the chunk is
$ %4 is the place on that page where the chunk is, %5 is size of user data
243  "    %%1$08x %%2$08x: (page %%3$d offset %%4$d) %%5$d bytes "
244  " *** NOT 8-BYTE ALIGNED *** "
245  "(free)"
246  " *** INVALID COOKIE *** "
$ 
$ NOTRANS
247  "\n"
$
$ The next 6 messages are DEVELOP.
$
248  "The version of idl_base.h is not compatible with this stub/switch code."
249  "The version of rpc.idl is not compatible with this stub/switch code."
250  "The version of ncastat.idl is not compatible with this stub/switch code."
251  "The version of idl_base.h is not compatible with this stub/switch code."
252  "The version of rpc.idl is not compatible with this stub/switch code."
253  "The version of ncastat.idl is not compatible with this stub/switch code."
$
$ ------------------------------- Global Location Broker ----------------------
$
$ The next 2 messages are ADMIN.
254  "cannot open replica - "
255  "cannot get socket - "
$
$ %1 is the name of the module, %2 is the glb object, %3 is the glb type
$ DEVELOP message.
256  "%%1$s opened  glb_object %%2$s  glb_type %%3$s\n"
$
$ The next 3 messages are ADMIN.
$
257  "cannot get glb uid - "
258  "cannot get glb type - "
259  "cannot create replica - "
$
$ %1 is the name of the module, %2 is the glb object, %3 is the glb type
$ DEVELOP message.
260  "%%1$s replica created  glb_object %%2$s  glb_type %%3$s\n"
$
$ The next 3 messages are ADMIN messages.
$
261  "DT received fault - "
262  "exiting\n"
263  "-version"
$
$ NOTRANS.
264  "%%1$s\n"
$
$ The next 3 messages are ADMIN.
265  "-obj"
266  "-o must be followed by object name\n"
267  "-o specified more than once\n"
268  "bad object name - "
$
$ The next 3 messages are NOTRANS.
$
269  "d"
270  "_log"
271  "-log_stdout"
$
$ ADMIN.
272  "cannot open log file\n"
$
$ NOTRANS
273  ".bak"
$
$ The next 9 messages are ADMIN.
$
274  "Command use: glbd [-create  -first [-family family_spec] | -from remote_replica_name] | [-change_family family_spec] | [-version] [-listen address_family_list]\n"
275  "-create"
276  "-first"
277  "-from"
278  "cannot get remote replica socket address - "
279  "-family"
280  "cannot get id of address family %%1$s - "
281  "-change_family"
282  "-listen"
$
$ NOTRANS.
283  "-"
$
$ ADMIN.
284  "No listen address families specified\n"
$
$ The next 3 messages are DEVELOP.
$
285  "-debug"
286  "-rpc_debug"
287  "-task_debug"
$
$ NOTRANS.
288  "\0"
$
$ The next 7 messages are ADMIN.
$
289  "cannot get id of listen address family %%1$s - "
290  "Warning: this host does not support the address family %%1$s.  Not listening for this family\n"
291  "<unknown>"
292  "Listening on required replica address family %%1$s\n"
293  "Unable to listen for %%1$s address family - "
294  "cannot rpc register - "
295  "Unable to register for %%1$s address family with local LB - "
$
$ The next 2 messages are DEVELOP.
$
296  "Add entry rqst"
297  "Del entry rqst"
$
$ The next 5 messages are NOTRANS.
$
298  "\0"
$
$ %1 is the action requested on object %2 with a %3 interface
299  "%%1$s   object %%2$s  if %%3$s "
300  "(GLB) "
301  "?(GLB) "
302  "\n"
$ 
$ The next 18 messages are DEVELOP.
303  "The version of idl_base.h is not compatible with this stub/switch code."
304  "The version of rpc.idl is not compatible with this stub/switch code."
305  "The version of ncastat.idl is not compatible with this stub/switch code."
306  "The version of idl_base.h is not compatible with this stub/switch code."
307  "The version of rpc.idl is not compatible with this stub/switch code."
308  "The version of ncastat.idl is not compatible with this stub/switch code."
309  "The version of idl_base.h is not compatible with this stub/switch code."
310  "The version of rpc.idl is not compatible with this stub/switch code."
311  "The version of ncastat.idl is not compatible with this stub/switch code."
312  "The version of idl_base.h is not compatible with this stub/switch code."
313  "The version of rpc.idl is not compatible with this stub/switch code."
314  "The version of ncastat.idl is not compatible with this stub/switch code."
315  "The version of idl_base.h is not compatible with this stub/switch code."
316  "The version of rpc.idl is not compatible with this stub/switch code."
317  "The version of ncastat.idl is not compatible with this stub/switch code."
318  "The version of idl_base.h is not compatible with this stub/switch code."
319  "The version of rpc.idl is not compatible with this stub/switch code."
320  "The version of ncastat.idl is not compatible with this stub/switch code."
$
$ ------------------------- Replicated Objected Name Management ---------------
$
$ The next 12 messages are ADMIN.
$
321  "glb"
322  "glb_test"
323  "nd"
324  "rgy"
325  "rgy_test"
326  "drm_test"
$
$ ------------------------------- Symbol Table Management ---------------------
$
327  "bad status on create %%1$x\n"
328  "bad status on allocate %%1$x\n"
329  "bad status on free %%1$x\n"
330  "bad status on close %%1$x\n"
331  "Unable to read version (%%1$d) file"
332  "Unable to get memory for file (%%1$d)"
$
$ The next 2 messages are DEVELOP.
$
333  "*** sym__print_table not compiled in.\n"
334  "*** empty table ***\n"
