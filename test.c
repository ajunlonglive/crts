# 1 "client/cfg/keymap.c"
# 1 "<built-in>"
# 1 "<command-line>"
# 31 "<command-line>"
# 1 "/usr/include/stdc-predef.h" 1 3 4
# 32 "<command-line>" 2
# 1 "client/cfg/keymap.c"
# 1 "include/posix.h" 1
# 2 "client/cfg/keymap.c" 2

# 1 "/usr/include/assert.h" 1 3 4
# 1 "/usr/include/features.h" 1 3 4
# 2 "/usr/include/assert.h" 2 3 4
# 19 "/usr/include/assert.h" 3 4

# 19 "/usr/include/assert.h" 3 4
_Noreturn void __assert_fail(const char *, const char *, int, const char *);
# 4 "client/cfg/keymap.c" 2
# 1 "/usr/include/stdlib.h" 1 3 4
# 19 "/usr/include/stdlib.h" 3 4
# 1 "/usr/include/bits/alltypes.h" 1 3 4
# 10 "/usr/include/bits/alltypes.h" 3 4
typedef int wchar_t;
# 50 "/usr/include/bits/alltypes.h" 3 4
typedef unsigned long size_t;
# 20 "/usr/include/stdlib.h" 2 3 4

int atoi(const char *);
long atol(const char *);
long long atoll(const char *);
double atof(const char *);

float strtof(const char *restrict, char **restrict);
double strtod(const char *restrict, char **restrict);
long double strtold(const char *restrict, char **restrict);

long strtol(const char *restrict, char **restrict, int);
unsigned long strtoul(const char *restrict, char **restrict, int);
long long strtoll(const char *restrict, char **restrict, int);
unsigned long long strtoull(const char *restrict, char **restrict, int);

int rand(void);
void srand(unsigned);

void *malloc(size_t);
void *calloc(size_t, size_t);
void *realloc(void *, size_t);
void free(void *);
void *aligned_alloc(size_t, size_t);

_Noreturn void abort(void);
int atexit(void (*)(void));
_Noreturn void exit(int);
_Noreturn void _Exit(int);
int at_quick_exit(void (*)(void));
_Noreturn void quick_exit(int);

char *getenv(const char *);

int system(const char *);

void *bsearch(const void *, const void *, size_t, size_t, int (*)(const void *, const void *));
void qsort(void *, size_t, size_t, int (*)(const void *, const void *));

int abs(int);
long labs(long);
long long llabs(long long);

typedef struct { int quot, rem; } div_t;
typedef struct { long quot, rem; } ldiv_t;
typedef struct { long long quot, rem; } lldiv_t;

div_t div(int, int);
ldiv_t ldiv(long, long);
lldiv_t lldiv(long long, long long);

int mblen(const char *, size_t);
int mbtowc(wchar_t *restrict, const char *restrict, size_t);
int wctomb(char *, wchar_t);
size_t mbstowcs(wchar_t *restrict, const char *restrict, size_t);
size_t wcstombs(char *restrict, const wchar_t *restrict, size_t);




size_t __ctype_get_mb_cur_max(void);
# 99 "/usr/include/stdlib.h" 3 4
int posix_memalign(void **, size_t, size_t);
int setenv(const char *, const char *, int);
int unsetenv(const char *);
int mkstemp(char *);
int mkostemp(char *, int);
char *mkdtemp(char *);
int getsubopt(char **, char *const *, char **);
int rand_r(unsigned *);
# 5 "client/cfg/keymap.c" 2
# 1 "/usr/include/string.h" 1 3 4
# 23 "/usr/include/string.h" 3 4
# 1 "/usr/include/bits/alltypes.h" 1 3 4
# 343 "/usr/include/bits/alltypes.h" 3 4
typedef struct __locale_struct * locale_t;
# 24 "/usr/include/string.h" 2 3 4

void *memcpy(void *restrict, const void *restrict, size_t);
void *memmove(void *, const void *, size_t);
void *memset(void *, int, size_t);
int memcmp(const void *, const void *, size_t);
void *memchr(const void *, int, size_t);

char *strcpy(char *restrict, const char *restrict);
char *strncpy(char *restrict, const char *restrict, size_t);

char *strcat(char *restrict, const char *restrict);
char *strncat(char *restrict, const char *restrict, size_t);

int strcmp(const char *, const char *);
int strncmp(const char *, const char *, size_t);

int strcoll(const char *, const char *);
size_t strxfrm(char *restrict, const char *restrict, size_t);

char *strchr(const char *, int);
char *strrchr(const char *, int);

size_t strcspn(const char *, const char *);
size_t strspn(const char *, const char *);
char *strpbrk(const char *, const char *);
char *strstr(const char *, const char *);
char *strtok(char *restrict, const char *restrict);

size_t strlen(const char *);

char *strerror(int);
# 63 "/usr/include/string.h" 3 4
char *strtok_r(char *restrict, const char *restrict, char **restrict);
int strerror_r(int, char *, size_t);
char *stpcpy(char *restrict, const char *restrict);
char *stpncpy(char *restrict, const char *restrict, size_t);
size_t strnlen(const char *, size_t);
char *strdup(const char *);
char *strndup(const char *, size_t);
char *strsignal(int);
char *strerror_l(int, locale_t);
int strcoll_l(const char *, const char *, locale_t);
size_t strxfrm_l(char *restrict, const char *restrict, size_t, locale_t);
# 6 "client/cfg/keymap.c" 2
# 1 "/usr/include/unistd.h" 1 3 4
# 33 "/usr/include/unistd.h" 3 4
# 1 "/usr/include/bits/alltypes.h" 1 3 4
# 65 "/usr/include/bits/alltypes.h" 3 4
typedef long ssize_t;




typedef long intptr_t;
# 162 "/usr/include/bits/alltypes.h" 3 4
typedef long off_t;
# 235 "/usr/include/bits/alltypes.h" 3 4
typedef int pid_t;
# 245 "/usr/include/bits/alltypes.h" 3 4
typedef unsigned uid_t;




typedef unsigned gid_t;
# 260 "/usr/include/bits/alltypes.h" 3 4
typedef unsigned useconds_t;
# 34 "/usr/include/unistd.h" 2 3 4

int pipe(int [2]);
int pipe2(int [2], int);
int close(int);
int posix_close(int, int);
int dup(int);
int dup2(int, int);
int dup3(int, int, int);
off_t lseek(int, off_t, int);
int fsync(int);
int fdatasync(int);

ssize_t read(int, void *, size_t);
ssize_t write(int, const void *, size_t);
ssize_t pread(int, void *, size_t, off_t);
ssize_t pwrite(int, const void *, size_t, off_t);

int chown(const char *, uid_t, gid_t);
int fchown(int, uid_t, gid_t);
int lchown(const char *, uid_t, gid_t);
int fchownat(int, const char *, uid_t, gid_t, int);

int link(const char *, const char *);
int linkat(int, const char *, int, const char *, int);
int symlink(const char *, const char *);
int symlinkat(const char *, int, const char *);
ssize_t readlink(const char *restrict, char *restrict, size_t);
ssize_t readlinkat(int, const char *restrict, char *restrict, size_t);
int unlink(const char *);
int unlinkat(int, const char *, int);
int rmdir(const char *);
int truncate(const char *, off_t);
int ftruncate(int, off_t);






int access(const char *, int);
int faccessat(int, const char *, int, int);

int chdir(const char *);
int fchdir(int);
char *getcwd(char *, size_t);

unsigned alarm(unsigned);
unsigned sleep(unsigned);
int pause(void);

pid_t fork(void);
int execve(const char *, char *const [], char *const []);
int execv(const char *, char *const []);
int execle(const char *, const char *, ...);
int execl(const char *, const char *, ...);
int execvp(const char *, char *const []);
int execlp(const char *, const char *, ...);
int fexecve(int, char *const [], char *const []);
_Noreturn void _exit(int);

pid_t getpid(void);
pid_t getppid(void);
pid_t getpgrp(void);
pid_t getpgid(pid_t);
int setpgid(pid_t, pid_t);
pid_t setsid(void);
pid_t getsid(pid_t);
char *ttyname(int);
int ttyname_r(int, char *, size_t);
int isatty(int);
pid_t tcgetpgrp(int);
int tcsetpgrp(int, pid_t);

uid_t getuid(void);
uid_t geteuid(void);
gid_t getgid(void);
gid_t getegid(void);
int getgroups(int, gid_t []);
int setuid(uid_t);
int seteuid(uid_t);
int setgid(gid_t);
int setegid(gid_t);

char *getlogin(void);
int getlogin_r(char *, size_t);
int gethostname(char *, size_t);
char *ctermid(char *);

int getopt(int, char * const [], const char *);
extern char *optarg;
extern int optind, opterr, optopt;

long pathconf(const char *, int);
long fpathconf(int, int);
long sysconf(int);
size_t confstr(int, char *, size_t);
# 254 "/usr/include/unistd.h" 3 4
# 1 "/usr/include/bits/posix.h" 1 3 4
# 255 "/usr/include/unistd.h" 2 3 4
# 7 "client/cfg/keymap.c" 2

# 1 "include/client/cfg/keymap.h" 1





# 1 "/usr/include/stdbool.h" 1 3 4
# 7 "include/client/cfg/keymap.h" 2
# 1 "/usr/include/stdint.h" 1 3 4
# 20 "/usr/include/stdint.h" 3 4
# 1 "/usr/include/bits/alltypes.h" 1 3 4
# 55 "/usr/include/bits/alltypes.h" 3 4
typedef unsigned long uintptr_t;
# 96 "/usr/include/bits/alltypes.h" 3 4
typedef signed char int8_t;




typedef signed short int16_t;




typedef signed int int32_t;




typedef signed long int64_t;




typedef signed long intmax_t;




typedef unsigned char uint8_t;




typedef unsigned short uint16_t;




typedef unsigned int uint32_t;




typedef unsigned long uint64_t;
# 146 "/usr/include/bits/alltypes.h" 3 4
typedef unsigned long uintmax_t;
# 21 "/usr/include/stdint.h" 2 3 4

typedef int8_t int_fast8_t;
typedef int64_t int_fast64_t;

typedef int8_t int_least8_t;
typedef int16_t int_least16_t;
typedef int32_t int_least32_t;
typedef int64_t int_least64_t;

typedef uint8_t uint_fast8_t;
typedef uint64_t uint_fast64_t;

typedef uint8_t uint_least8_t;
typedef uint16_t uint_least16_t;
typedef uint32_t uint_least32_t;
typedef uint64_t uint_least64_t;
# 95 "/usr/include/stdint.h" 3 4
# 1 "/usr/include/bits/stdint.h" 1 3 4
typedef int32_t int_fast16_t;
typedef int32_t int_fast32_t;
typedef uint32_t uint_fast16_t;
typedef uint32_t uint_fast32_t;
# 96 "/usr/include/stdint.h" 2 3 4
# 8 "include/client/cfg/keymap.h" 2

# 1 "include/client/input/keymap.h" 1







# 1 "/usr/include/stddef.h" 1 3 4
# 17 "/usr/include/stddef.h" 3 4
# 1 "/usr/include/bits/alltypes.h" 1 3 4
# 41 "/usr/include/bits/alltypes.h" 3 4
typedef struct { long long __ll; long double __ld; } max_align_t;
# 60 "/usr/include/bits/alltypes.h" 3 4
typedef long ptrdiff_t;
# 18 "/usr/include/stddef.h" 2 3 4
# 9 "include/client/input/keymap.h" 2


# 10 "include/client/input/keymap.h"
enum key_command {
	kc_none,
	kc_center,
	kc_center_cursor,
	kc_macro,
	kc_invalid,
	kc_view_left,
	kc_view_down,
	kc_view_up,
	kc_view_right,
	kc_find,
	kc_set_input_mode,
	kc_quit,
	kc_cursor_left,
	kc_cursor_down,
	kc_cursor_up,
	kc_cursor_right,
	kc_set_action_type,
	kc_set_action_target,
	kc_toggle_action_flag,
	kc_read_action_target,
	kc_undo_action,
	kc_swap_cursor_with_source,
	kc_set_action_height,
	kc_action_height_grow,
	kc_action_height_shrink,
	kc_set_action_width,
	kc_action_width_grow,
	kc_action_width_shrink,
	kc_action_rect_rotate,
	kc_exec_action,
	kc_toggle_help,
	key_command_count
};

enum special_keycodes {
	skc_up = 1,
	skc_down = 2,
	skc_left = 3,
	skc_right = 4,
	skc_f1 = 5,
	skc_f2 = 6,
	skc_f3 = 7,
	skc_f4 = 8,
	skc_f5 = 9,
	skc_f6 = 10,
	skc_f7 = 11,
	skc_f8 = 12,
	skc_f9 = 13,
	skc_f10 = 14,
	skc_f11 = 15,
	skc_f12 = 16,
};

enum input_mode {
	im_normal,
	im_select,
	im_resize,
	im_cmd,
	im_none,
	im_invalid,
	input_mode_count = 4
};

extern const char *input_mode_names[input_mode_count];

struct keymap {
	char trigger[32];
	char strcmd[32];
	char desc[64 + 1];
	struct keymap *map;
	enum key_command cmd;
};

enum keymap_category {
	kmc_dont_use = 0,
	kmc_nav,
	kmc_resize,
	kmc_act_conf,
	kmc_act_ctrl,
	kmc_sys,
};

enum keymap_hook_result {
	khr_failed,
	khr_unmatched,
	khr_matched
};

void keymap_init(struct keymap *km);
# 10 "include/client/cfg/keymap.h" 2
# 1 "include/client/ui/common.h" 1



# 1 "include/client/hiface.h" 1





# 1 "include/client/input/cmdline.h" 1
# 12 "include/client/input/cmdline.h"
enum cmd_result {
	cmdres_ok,
	cmdres_not_found,
	cmdres_arg_error,
	cmdres_cmd_error,
};

struct cmd_ctx {
	char cmdline[256];
	char *argv[32];
	uint32_t argc;
	char out[256];
};

typedef enum cmd_result ((*cmdfunc)(struct cmd_ctx *cmd, void *ctx));

struct cmd_table {
	char *cmd;
	cmdfunc action;
};

struct cmdline_buf {
	char buf[256];
	size_t len, cursor;
};


struct cmdline {
	struct cmdline_buf cur, tmp;
	struct {
		char in[32][256];
		char out[32][256];
		size_t len, cursor;
	} history;
};

struct hiface;

void parse_cmd_input(struct hiface *hf, unsigned k);
cmdfunc cmd_lookup(const struct cmd_ctx *cmd, const struct cmd_table *tbl, size_t tbl_len);
void run_cmd_string(struct hiface *hf, const char *cmds);
# 7 "include/client/hiface.h" 2

# 1 "include/client/sim.h" 1







# 1 "include/shared/sim/action.h" 1







# 1 "include/shared/math/geom.h" 1




# 1 "include/shared/types/geom.h" 1





struct point {
	int x;
	int y;
};

struct circle {
	struct point center;
	int r;
};

struct rectangle {
	struct point pos;
	int width;
	int height;
};

struct pointf {
	float x, y;
};

typedef float line[3];
# 6 "include/shared/math/geom.h" 2




int points_equal(const struct point *a, const struct point *b);
int points_adjacent(const struct point *a, const struct point *b);
int point_in_circle(const struct point *p, const struct circle *c);
int point_in_rect(const struct point *p, const struct rectangle *r);
struct point point_mod(const struct point *p, int operand);
int distance_point_to_circle(const struct point *p, const struct circle *c);
int dot(const struct point a, const struct point b);
struct point point_sub(const struct point *a, const struct point *b);
struct point point_add(const struct point *a, const struct point *b);
int square_dist(const struct point *a, const struct point *b);
uint32_t rect_area(const struct rectangle *rect);

float fsqdist(const struct pointf *p, const struct pointf *q);
void make_line(struct pointf *p, struct pointf *q, line l);
float signed_area(const struct pointf *v0, const struct pointf *v1, const struct pointf *v2);

# 25 "include/shared/math/geom.h" 3 4
_Bool
# 25 "include/shared/math/geom.h"
intersection_of(line l1, line l2, struct pointf *p);
void make_perpendicular_bisector(struct pointf *p, struct pointf *q, line l);
float nearest_neighbour(float a, float b, float c, float d, float x, float y);
# 9 "include/shared/sim/action.h" 2




enum action_type {
	at_none,
	at_move,
	at_harvest,
	at_build,
	at_fight,
	at_carry,
	action_type_count
};

enum action_flags {
	af_repeat = 1 << 0,
	action_flags_count = 1
};

struct action {
	struct rectangle range;
	struct rectangle source;
	enum action_type type;
	uint16_t tgt;
	uint16_t workers_requested;
	uint8_t id;
	uint8_t flags;







};

void action_init(struct action *act);
void action_inspect(struct action *act);
# 9 "include/client/sim.h" 2



# 1 "include/client/opts.h" 1





struct c_opts {
	uint8_t ui;
	uint16_t id;
	const char *ip_addr;
	const char *load_map;
	const char *cmds;
};

void process_c_opts(int argc, char * const *argv, struct c_opts *opts);
# 13 "include/client/sim.h" 2
struct c_simulation {
	struct queue *outbound;
	struct queue *inbound;
	struct world *w;

	struct {
		size_t ents;
	} server_world;

	struct {

# 23 "include/client/sim.h" 3 4
		_Bool
# 23 "include/client/sim.h"
		chunks;

# 24 "include/client/sim.h" 3 4
		_Bool
# 24 "include/client/sim.h"
		ents;

# 25 "include/client/sim.h" 3 4
		_Bool
# 25 "include/client/sim.h"
		actions;
	} changed;

	struct action action_history[256];
	uint8_t action_history_order[256];
	size_t action_history_len;

	uint16_t id;

	int run;
};
# 9 "include/client/hiface.h" 2
# 1 "include/shared/net/net_ctx.h" 1



# 1 "include/shared/net/msg_queue.h" 1





# 1 "include/shared/net/defs.h" 1



# 1 "/usr/include/arpa/inet.h" 1 3 4
# 9 "/usr/include/arpa/inet.h" 3 4
# 1 "/usr/include/netinet/in.h" 1 3 4
# 9 "/usr/include/netinet/in.h" 3 4
# 1 "/usr/include/inttypes.h" 1 3 4
# 12 "/usr/include/inttypes.h" 3 4
# 1 "/usr/include/bits/alltypes.h" 1 3 4
# 13 "/usr/include/inttypes.h" 2 3 4


# 14 "/usr/include/inttypes.h" 3 4
typedef struct { intmax_t quot, rem; } imaxdiv_t;

intmax_t imaxabs(intmax_t);
imaxdiv_t imaxdiv(intmax_t, intmax_t);

intmax_t strtoimax(const char *restrict, char **restrict, int);
uintmax_t strtoumax(const char *restrict, char **restrict, int);

intmax_t wcstoimax(const wchar_t *restrict, wchar_t **restrict, int);
uintmax_t wcstoumax(const wchar_t *restrict, wchar_t **restrict, int);
# 10 "/usr/include/netinet/in.h" 2 3 4
# 1 "/usr/include/sys/socket.h" 1 3 4
# 18 "/usr/include/sys/socket.h" 3 4
# 1 "/usr/include/bits/alltypes.h" 1 3 4
# 355 "/usr/include/bits/alltypes.h" 3 4
struct iovec { void *iov_base; size_t iov_len; };





typedef unsigned socklen_t;




typedef unsigned short sa_family_t;
# 19 "/usr/include/sys/socket.h" 2 3 4

# 1 "/usr/include/bits/socket.h" 1 3 4
# 21 "/usr/include/sys/socket.h" 2 3 4

struct msghdr {
	void *msg_name;
	socklen_t msg_namelen;
	struct iovec *msg_iov;



	int msg_iovlen;

	int __pad1;

	void *msg_control;



	socklen_t msg_controllen;

	int __pad2;

	int msg_flags;
};

struct cmsghdr {



	socklen_t cmsg_len;

	int __pad1;

	int cmsg_level;
	int cmsg_type;
};
# 74 "/usr/include/sys/socket.h" 3 4
struct linger {
	int l_onoff;
	int l_linger;
};
# 367 "/usr/include/sys/socket.h" 3 4
struct sockaddr {
	sa_family_t sa_family;
	char sa_data[14];
};

struct sockaddr_storage {
	sa_family_t ss_family;
	char __ss_padding[128 - sizeof(long) - sizeof(sa_family_t)];
	unsigned long __ss_align;
};

int socket(int, int, int);
int socketpair(int, int, int, int [2]);

int shutdown(int, int);

int bind(int, const struct sockaddr *, socklen_t);
int connect(int, const struct sockaddr *, socklen_t);
int listen(int, int);
int accept(int, struct sockaddr *restrict, socklen_t *restrict);
int accept4(int, struct sockaddr *restrict, socklen_t *restrict, int);

int getsockname(int, struct sockaddr *restrict, socklen_t *restrict);
int getpeername(int, struct sockaddr *restrict, socklen_t *restrict);

ssize_t send(int, const void *, size_t, int);
ssize_t recv(int, void *, size_t, int);
ssize_t sendto(int, const void *, size_t, int, const struct sockaddr *, socklen_t);
ssize_t recvfrom(int, void *restrict, size_t, int, struct sockaddr *restrict, socklen_t *restrict);
ssize_t sendmsg(int, const struct msghdr *, int);
ssize_t recvmsg(int, struct msghdr *, int);

int getsockopt(int, int, int, void *restrict, socklen_t *restrict);
int setsockopt(int, int, int, const void *, socklen_t);

int sockatmark(int);
# 11 "/usr/include/netinet/in.h" 2 3 4

typedef uint16_t in_port_t;
typedef uint32_t in_addr_t;
struct in_addr { in_addr_t s_addr; };

struct sockaddr_in {
	sa_family_t sin_family;
	in_port_t sin_port;
	struct in_addr sin_addr;
	uint8_t sin_zero[8];
};

struct in6_addr {
	union {
		uint8_t __s6_addr[16];
		uint16_t __s6_addr16[8];
		uint32_t __s6_addr32[4];
	} __in6_union;
};




struct sockaddr_in6 {
	sa_family_t sin6_family;
	in_port_t sin6_port;
	uint32_t sin6_flowinfo;
	struct in6_addr sin6_addr;
	uint32_t sin6_scope_id;
};

struct ipv6_mreq {
	struct in6_addr ipv6mr_multiaddr;
	unsigned ipv6mr_interface;
};
# 61 "/usr/include/netinet/in.h" 3 4
extern const struct in6_addr in6addr_any, in6addr_loopback;




uint32_t htonl(uint32_t);
uint16_t htons(uint16_t);
uint32_t ntohl(uint32_t);
uint16_t ntohs(uint16_t);
# 229 "/usr/include/netinet/in.h" 3 4
struct ip_opts {
	struct in_addr ip_dst;
	char ip_opts[40];
};
# 10 "/usr/include/arpa/inet.h" 2 3 4

uint32_t htonl(uint32_t);
uint16_t htons(uint16_t);
uint32_t ntohl(uint32_t);
uint16_t ntohs(uint16_t);

in_addr_t inet_addr(const char *);
in_addr_t inet_network(const char *);
char *inet_ntoa(struct in_addr);
int inet_pton(int, const char *restrict, void *restrict);
const char *inet_ntop(int, const void *restrict, char *restrict, socklen_t);

int inet_aton(const char *, struct in_addr *);
struct in_addr inet_makeaddr(in_addr_t, in_addr_t);
in_addr_t inet_lnaof(struct in_addr);
in_addr_t inet_netof(struct in_addr);
# 5 "include/shared/net/defs.h" 2
# 16 "include/shared/net/defs.h"

# 16 "include/shared/net/defs.h"
extern socklen_t socklen;

typedef uint16_t msg_seq_t;
typedef uint32_t msg_ack_t;
typedef uint32_t cx_bits_t;

enum msg_flags {
	msgf_forget = 1 << 0,
	msgf_drop_if_full = 1 << 1,
	msgf_must_send = 1 << 2,
	msgf_ack = 1 << 3,
	msg_flags_max = msgf_forget
			| msgf_drop_if_full
			| msgf_must_send
			| msgf_ack
};


struct msg_hello {
	uint8_t version[12];
	uint16_t id;
};

enum msg_kind {
	mk_msg,
	mk_ack,
	mk_hello,
	msg_kind_count,
};

struct msg_hdr {
	enum msg_kind kind;
	msg_seq_t seq;
};
# 7 "include/shared/net/msg_queue.h" 2
# 1 "include/shared/net/ack.h" 1






# 1 "include/shared/types/hash.h" 1






# 1 "include/shared/types/iterator.h" 1


enum iteration_result {
	ir_cont,
	ir_done
};

typedef enum iteration_result (*iterator_func)(void *ctx, void *val);
# 8 "include/shared/types/hash.h" 2

struct hash;

typedef enum iteration_result ((*hash_with_keys_iterator_func)(void *ctx, void *key, size_t val));

const size_t *hash_get(const struct hash *h, const void *key);
struct hash *hash_init(size_t buckets, size_t bdepth, size_t keysize);
void hash_destroy(struct hash *h);
void hash_for_each(struct hash *h, void *ctx, iterator_func ifnc);
void hash_for_each_with_keys(struct hash *h, void *ctx, hash_with_keys_iterator_func ifnc);
void hash_set(struct hash *h, const void *key, size_t val);
void hash_unset(struct hash *h, const void *key);
size_t hash_len(const struct hash *h);
void hash_clear(struct hash *h);
void hash_inspect(const struct hash *h);
# 8 "include/shared/net/ack.h" 2


struct msg_queue;



typedef uint32_t ack_t;

struct ack_group {
	msg_seq_t leader;
	ack_t acks;
};

typedef enum iteration_result (*ack_iter_func)(void *, msg_seq_t);

struct hash * ack_init(void);
void ack_set(struct hash *ags, msg_seq_t new);
void ack_clear_all(struct hash *ags);

# 26 "include/shared/net/ack.h" 3 4
_Bool
# 26 "include/shared/net/ack.h"
ack_check(struct hash *ags, msg_seq_t id);
void ack_msgq(struct hash *ags, struct msg_queue *q, cx_bits_t acker);
# 8 "include/shared/net/msg_queue.h" 2

typedef void ((*msgq_send_all_iter)(void *, cx_bits_t, msg_seq_t, enum msg_flags, void *, uint16_t));

struct msg_queue;
struct message;

struct msg_queue *msgq_init(void);
void msgq_add(struct msg_queue *q, struct message *msg, cx_bits_t send_to,
	enum msg_flags flags);
void msgq_ack(struct msg_queue *q, msg_seq_t seq, cx_bits_t acker);
void msgq_send_all(struct msg_queue *q, void *ctx, msgq_send_all_iter);
void msgq_compact(struct msg_queue *q);
# 5 "include/shared/net/net_ctx.h" 2
# 1 "include/shared/net/pool.h" 1





struct cx_pool {
	struct hdarr *cxs;
	cx_bits_t cx_bits;
};

void cx_pool_init(struct cx_pool *);
void cx_prune(struct cx_pool *, long ms);
struct connection *cx_establish(struct cx_pool *cp, struct sockaddr_in *addr);
struct connection *cx_add(struct cx_pool *cp, struct sockaddr_in *addr, uint16_t id);
void cx_pool_clear(struct cx_pool *cp);
# 6 "include/shared/net/net_ctx.h" 2
# 1 "include/shared/net/recv_msgs.h" 1



# 1 "include/shared/net/net_ctx.h" 1
# 5 "include/shared/net/recv_msgs.h" 2

struct net_ctx;

void recv_msgs(struct net_ctx *ctx);
# 7 "include/shared/net/net_ctx.h" 2
# 1 "include/shared/net/send_msgs.h" 1






struct net_ctx;
void send_msgs(struct net_ctx *nx);
# 8 "include/shared/net/net_ctx.h" 2
# 1 "include/shared/serialize/message.h" 1





# 1 "include/shared/serialize/chunk.h" 1



# 1 "include/shared/serialize/coder.h" 1






struct ac_coder {
	uint64_t lim;
	uint32_t ceil, floor, pending;

	uint8_t *buf;
	uint32_t bufi, buflen;
};

struct ac_decoder {
	uint64_t lim;
	uint32_t ceil, floor, val;

	const uint8_t *buf;
	uint32_t bufi, buflen, bufused;
};

void ac_pack_init(struct ac_coder *c, uint8_t *buf, size_t blen);
void ac_pack(struct ac_coder *c, uint32_t val);
void ac_pack_finish(struct ac_coder *c);
void ac_unpack_init(struct ac_decoder *c, const uint8_t *buf, size_t blen);
void ac_unpack(struct ac_decoder *c, uint32_t out[], size_t len);
size_t ac_coder_len(struct ac_coder *c);
size_t ac_decoder_len(struct ac_decoder *c);
# 5 "include/shared/serialize/chunk.h" 2
# 1 "include/shared/sim/chunk.h" 1
# 9 "include/shared/sim/chunk.h"
# 1 "include/shared/types/darr.h" 1






struct darr;

size_t darr_push(struct darr *da, const void *item);
struct darr *darr_init(size_t item_size);
void *darr_try_get(const struct darr *da, size_t i);
void *darr_get(const struct darr *da, size_t i);
void darr_del(struct darr *da, size_t i);
void darr_destroy(struct darr *da);
void darr_for_each(struct darr *da, void *ctx, iterator_func ifnc);
void darr_set(struct darr *da, size_t i, const void *item);
size_t darr_len(const struct darr *da);
void darr_clear(struct darr *da);

size_t darr_item_size(const struct darr *da);
size_t darr_size(const struct darr *da);
void *darr_raw_memory(const struct darr *da);
char *darr_point_at(const struct darr *da, size_t i);
void *darr_get_mem(struct darr *da);
void darr_grow_to(struct darr *da, size_t size);
void darr_clear_iter(struct darr *da, void *ctx, iterator_func ifnc);

void darr_swap(struct darr *da, size_t i, size_t j);
# 10 "include/shared/sim/chunk.h" 2
# 1 "include/shared/types/hdarr.h" 1
# 9 "include/shared/types/hdarr.h"
typedef const void *(*hdarr_key_getter)(void *elem);

struct hdarr;

struct hdarr *hdarr_init(size_t size, size_t keysize, size_t item_size, hdarr_key_getter kg);
void *hdarr_get(struct hdarr *hd, const void *key);
void hdarr_del(struct hdarr *hd, const void *key);
const size_t *hdarr_get_i(struct hdarr *hd, const void *key);
void *hdarr_get_by_i(struct hdarr *hd, size_t i);
void hdarr_destroy(struct hdarr *hd);
void hdarr_for_each(struct hdarr *hd, void *ctx, iterator_func ifnc);
size_t hdarr_set(struct hdarr *hd, const void *key, const void *value);
void hdarr_reset(struct hdarr *hd, const void *okey, const void *nkey);
size_t hdarr_len(const struct hdarr *hd);
void hdarr_clear(struct hdarr *hd);
struct darr *hdarr_darr(struct hdarr *hd);
# 11 "include/shared/sim/chunk.h" 2
# 20 "include/shared/sim/chunk.h"
enum tile {
	tile_deep_water,
	tile_water,
	tile_wetland,
	tile_plain,
	tile_forest,
	tile_mountain,
	tile_peak,
	tile_dirt,
	tile_forest_young,
	tile_forest_old,
	tile_wetland_forest_young,
	tile_wetland_forest,
	tile_wetland_forest_old,
	tile_coral,
	tile_stream,

	tile_wood,
	tile_stone,
	tile_wood_floor,
	tile_rock_floor,
	tile_farmland_empty,
	tile_farmland_done,
	tile_burning,
	tile_burnt,
	tile_storehouse,

	tile_count,
};

enum tile_function {
	tfunc_none,
	tfunc_dynamic,
	tfunc_storage,
};
# 70 "include/shared/sim/chunk.h"
struct chunk {
	uint32_t tiles[16][16];
	float heights[16][16];






	struct point pos;
};

struct chunks {




	struct hdarr *hd;







	size_t chunk_date;
};




_Static_assert(sizeof(size_t) == 8, "wrong size size_t");
# 116 "include/shared/sim/chunk.h"
void chunks_init(struct chunks *cnks);
struct point nearest_chunk(const struct point *p);
void chunks_destroy(struct chunks *cnks);
struct chunk *get_chunk(struct chunks *cnks, const struct point *p);
struct chunk *get_chunk_at(struct chunks *cnks, const struct point *p);
void set_chunk(struct chunks *cnks, struct chunk *ck);
# 6 "include/shared/serialize/chunk.h" 2

struct ser_chunk {
	struct point cp;
	enum tile tiles[16 * 16];
	float heights[16 * 16];
};

void fill_ser_chunk(struct ser_chunk *sck, const struct chunk *ck);
void unfill_ser_chunk(const struct ser_chunk *sck, struct chunk *ck);
void pack_ser_chunk(struct ac_coder *cod, const struct ser_chunk *sck);
void unpack_ser_chunk(struct ac_decoder *dec, struct ser_chunk *sck);
size_t unpack_chunk(struct chunk *ck, const uint8_t *buf, size_t blen);
size_t pack_chunk(const struct chunk *ck, uint8_t *buf, size_t blen);
# 7 "include/shared/serialize/message.h" 2


# 1 "include/shared/sim/ent.h" 1







# 1 "include/shared/sim/world.h" 1
# 13 "include/shared/sim/world.h"
struct world {
	struct chunks chunks;
	struct hdarr *ents;




	uint32_t seq;
};

struct world *world_init(void);
void world_despawn(struct world *w, uint32_t id);
# 9 "include/shared/sim/ent.h" 2

enum ent_type {
	et_none,
	et_worker,
	et_elf_corpse,
	et_deer,
	et_fish,
	et_vehicle_boat,
	et_resource_wood,
	et_resource_meat,
	et_resource_rock,
	et_resource_crop,
	et_storehouse,
	ent_type_count
};






enum ent_states {
	es_have_subtask = 1 << 0,
	es_have_task = 1 << 1,
	es_waiting = 1 << 2,
	es_killed = 1 << 3,
	es_modified = 1 << 4,
	es_in_storage = 1 << 5,
	es_hungry = 1 << 6,
	es_spawned = 1 << 7,
};

typedef uint32_t ent_id_t;

struct ent {
	struct point pos;

	ent_id_t id;
	enum ent_type type;
	uint16_t alignment;
	uint8_t damage;
# 64 "include/shared/sim/ent.h"
};

void ent_init(struct ent *e);

typedef
# 68 "include/shared/sim/ent.h" 3 4
	_Bool
# 68 "include/shared/sim/ent.h"
(*find_ent_predicate)(void *ctx, struct ent *e);
struct ent *find_ent(const struct world *w, const struct point *p, void *ctx,
	find_ent_predicate epred);
# 10 "include/shared/serialize/message.h" 2

enum message_type {
	mt_poke,
	mt_req,
	mt_ent,
	mt_action,
	mt_tile,
	mt_chunk,
	message_type_count
};

enum req_message_type {
	rmt_chunk,
	req_message_type_count,
};

enum action_message_type {
	amt_add,
	amt_del,
	action_message_type_count,
};

enum ent_message_type {
	emt_spawn,
	emt_pos,
	emt_kill,
	ent_message_type_count,
};

struct msg_req {
	enum req_message_type mt;
	union {
		struct point chunk;
	} dat;
};

struct msg_ent {
	enum ent_message_type mt;
	uint16_t id;
	union {
		struct point pos;
		struct {
			enum ent_type type;
			uint16_t alignment;
			struct point pos;
		} spawn;
	} dat;
};

struct msg_action {
	enum action_message_type mt;
	uint8_t id;
	union {
		struct {
			enum action_type type;
			uint16_t tgt;
			struct rectangle range;
		} add;
	} dat;
};

struct msg_tile {
	struct point cp;
	uint8_t c;
	float height;
	enum tile t;
};

struct msg_chunk {
	struct ser_chunk dat;
};

enum message_batch_size {
	mbs_req = 171,
	mbs_ent = 85,
	mbs_action = 64,
	mbs_tile = 102,
	mbs_chunk = 1
};

struct message {
	union {
		struct msg_req req[mbs_req];
		struct msg_ent ent[mbs_ent];
		struct msg_action action[mbs_action];
		struct msg_tile tile[mbs_tile];
		struct msg_chunk chunk[mbs_chunk];
	} dat;
	enum message_type mt;
	uint8_t count;
};
_Static_assert(sizeof(struct message) <= sizeof(struct chunk) + sizeof(enum message_type) + 8, "message batch size too big");

typedef void ((*msg_cb)(void *ctx, enum message_type, void *msg));

size_t pack_message(const struct message *msg, uint8_t *buf, uint32_t blen);
void unpack_message(uint8_t *buf, uint32_t blen, msg_cb cb, void *ctx);

# 107 "include/shared/serialize/message.h" 3 4
_Bool
# 107 "include/shared/serialize/message.h"
append_msg(struct message *msg, void *smsg);
const char *inspect_message(enum message_type mt, const void *msg);
# 9 "include/shared/net/net_ctx.h" 2

struct net_ctx;

typedef void ((*message_handler)(struct net_ctx *, enum message_type mt, void *msg, struct connection *cx));

struct net_ctx {
	void *usr_ctx;
	uint16_t id;

	message_handler handler;

	int sock;

	struct cx_pool cxs;
	struct msg_queue *send;

	struct {
		struct message msg;
		cx_bits_t dest;
		enum msg_flags f;
	} buf;
};

struct net_ctx *net_ctx_init(uint32_t port, uint32_t addr,
	message_handler handler, uint16_t id);
void queue_msg(struct net_ctx *nx, enum message_type mt, void *msg, cx_bits_t dest,
	enum msg_flags f);
# 10 "include/client/hiface.h" 2
# 19 "include/client/hiface.h"
struct hiface_buf {
	char buf[32];
	size_t len;
};

struct hiface {

	struct hiface_buf num;
	struct {

# 28 "include/client/hiface.h" 3 4
		_Bool
# 28 "include/client/hiface.h"
		override;
		long val;
	} num_override;
	struct hiface_buf cmd;
	struct cmdline cmdline;

	struct point cursor;
	struct point view;
	enum input_mode im;
	struct keymap km[input_mode_count];
	uint32_t redrew_world;

	struct action next_act;

# 41 "include/client/hiface.h" 3 4
	_Bool
# 41 "include/client/hiface.h"
	next_act_changed;
	uint8_t action_seq;


# 44 "include/client/hiface.h" 3 4
	_Bool
# 44 "include/client/hiface.h"
	keymap_describe;
	char description[64];
	size_t desc_len;

# 47 "include/client/hiface.h" 3 4
	_Bool
# 47 "include/client/hiface.h"
	input_changed;


# 49 "include/client/hiface.h" 3 4
	_Bool
# 49 "include/client/hiface.h"
	center_cursor;

# 50 "include/client/hiface.h" 3 4
	_Bool
# 50 "include/client/hiface.h"
	display_help;


	struct c_simulation *sim;
	struct net_ctx *nx;
	struct ui_ctx *ui_ctx;
# 65 "include/client/hiface.h"
};

struct hiface *hiface_init(struct c_simulation *sim);
long hiface_get_num(struct hiface *hif, long def);
void commit_action(struct hiface *hif);
void undo_action(struct hiface *hif);
void override_num_arg(struct hiface *hf, long num);
void hf_describe(struct hiface *hf, enum keymap_category cat, char *desc, ...);
void hiface_reset_input(struct hiface *hf);
void hifb_append_char(struct hiface_buf *hbf, unsigned c);
# 5 "include/client/ui/common.h" 2




enum ui_types {
	ui_null = 0,
	ui_ncurses = 1 << 0,
	ui_opengl = 1 << 1,
	ui_default = 1 << 7,
};

struct ui_ctx {
	struct ncurses_ui_ctx *ncurses;
	struct opengl_ui_ctx *opengl;
	uint8_t enabled;
};

void ui_init(struct c_opts *opts, struct ui_ctx *ctx);
void ui_render(struct ui_ctx *nc, struct hiface *hf);
void ui_handle_input(struct ui_ctx *ctx, struct keymap **km, struct hiface *hf);
struct rectangle ui_viewport(struct ui_ctx *nc);
void ui_deinit(struct ui_ctx *ctx);
enum cmd_result ui_cmdline_hook(struct cmd_ctx *cmd, struct ui_ctx *ctx, struct
	hiface *hf);
enum keymap_hook_result ui_keymap_hook(struct ui_ctx *ctx, struct keymap *km,
	const char *sec, const char *k, const char *v, uint32_t line);
# 11 "include/client/cfg/keymap.h" 2


# 12 "include/client/cfg/keymap.h" 3 4
_Bool
# 12 "include/client/cfg/keymap.h"
parse_keymap(struct keymap *km, struct ui_ctx *ui_ctx);
# 9 "client/cfg/keymap.c" 2
# 1 "include/client/input/handler.h" 1





typedef void (*kc_func)(struct hiface *);
typedef void ((*for_each_completion_cb)(void *ctx, struct keymap *km));

struct keymap *handle_input(struct keymap *km, unsigned k, struct hiface *hif);
void trigger_cmd(kc_func func, struct hiface *hf);
void for_each_completion(struct keymap *km, void *ctx, for_each_completion_cb cb);
void describe_completions(struct hiface *hf, struct keymap *km,
	void *usr_ctx, for_each_completion_cb cb);
# 10 "client/cfg/keymap.c" 2



# 1 "include/shared/util/inih.h" 1
# 12 "include/shared/util/inih.h"
# 1 "include/shared/util/assets.h" 1






struct file_data { const char *path; const uint8_t *data; size_t len; };

void asset_path_init(char *asset_path);
struct file_data* asset(const char *path);
const char *rel_to_abs_path(const char *relpath);
# 13 "include/shared/util/inih.h" 2

typedef
# 14 "include/shared/util/inih.h" 3 4
	_Bool
# 14 "include/shared/util/inih.h"
	((*inihcb)(void *ctx, char err[256], const char *sect, const char *k,
		   const char *v, uint32_t line));


# 17 "include/shared/util/inih.h" 3 4
_Bool
# 17 "include/shared/util/inih.h"
ini_parse(struct file_data *fd, inihcb cb, void *ctx);

struct cfg_lookup_table {
	struct {
		char *str;
		uint32_t t;
	} e[64];
};

int32_t cfg_string_lookup(const char *str, const struct cfg_lookup_table *tbl);

# 27 "include/shared/util/inih.h" 3 4
_Bool
# 27 "include/shared/util/inih.h"
parse_cfg_file(const char *filename, void *ctx, inihcb handler);

# 28 "include/shared/util/inih.h" 3 4
_Bool
# 28 "include/shared/util/inih.h"
str_to_bool(const char *str);
float strdeg_to_rad(const char *str);
# 14 "client/cfg/keymap.c" 2
# 1 "include/shared/util/log.h" 1



# 1 "include/posix.h" 1
# 5 "include/shared/util/log.h" 2

# 1 "/usr/include/assert.h" 1 3 4
# 19 "/usr/include/assert.h" 3 4

# 19 "/usr/include/assert.h" 3 4
_Noreturn void __assert_fail(const char *, const char *, int, const char *);
# 7 "include/shared/util/log.h" 2


# 1 "/usr/include/stdio.h" 1 3 4
# 26 "/usr/include/stdio.h" 3 4
# 1 "/usr/include/bits/alltypes.h" 1 3 4
# 320 "/usr/include/bits/alltypes.h" 3 4
typedef struct _IO_FILE FILE;





typedef __builtin_va_list va_list;




typedef __builtin_va_list __isoc_va_list;
# 27 "/usr/include/stdio.h" 2 3 4
# 54 "/usr/include/stdio.h" 3 4
typedef union _G_fpos64_t {
	char __opaque[16];
	long long __lldata;
	double __align;
} fpos_t;

extern FILE *const stdin;
extern FILE *const stdout;
extern FILE *const stderr;





FILE *fopen(const char *restrict, const char *restrict);
FILE *freopen(const char *restrict, const char *restrict, FILE *restrict);
int fclose(FILE *);

int remove(const char *);
int rename(const char *, const char *);

int feof(FILE *);
int ferror(FILE *);
int fflush(FILE *);
void clearerr(FILE *);

int fseek(FILE *, long, int);
long ftell(FILE *);
void rewind(FILE *);

int fgetpos(FILE *restrict, fpos_t *restrict);
int fsetpos(FILE *, const fpos_t *);

size_t fread(void *restrict, size_t, size_t, FILE *restrict);
size_t fwrite(const void *restrict, size_t, size_t, FILE *restrict);

int fgetc(FILE *);
int getc(FILE *);
int getchar(void);
int ungetc(int, FILE *);

int fputc(int, FILE *);
int putc(int, FILE *);
int putchar(int);

char *fgets(char *restrict, int, FILE *restrict);




int fputs(const char *restrict, FILE *restrict);
int puts(const char *);

int printf(const char *restrict, ...);
int fprintf(FILE *restrict, const char *restrict, ...);
int sprintf(char *restrict, const char *restrict, ...);
int snprintf(char *restrict, size_t, const char *restrict, ...);

int vprintf(const char *restrict, __isoc_va_list);
int vfprintf(FILE *restrict, const char *restrict, __isoc_va_list);
int vsprintf(char *restrict, const char *restrict, __isoc_va_list);
int vsnprintf(char *restrict, size_t, const char *restrict, __isoc_va_list);

int scanf(const char *restrict, ...);
int fscanf(FILE *restrict, const char *restrict, ...);
int sscanf(const char *restrict, const char *restrict, ...);
int vscanf(const char *restrict, __isoc_va_list);
int vfscanf(FILE *restrict, const char *restrict, __isoc_va_list);
int vsscanf(const char *restrict, const char *restrict, __isoc_va_list);

void perror(const char *);

int setvbuf(FILE *restrict, char *restrict, int, size_t);
void setbuf(FILE *restrict, char *restrict);

char *tmpnam(char *);
FILE *tmpfile(void);




FILE *fmemopen(void *restrict, size_t, const char *restrict);
FILE *open_memstream(char **, size_t *);
FILE *fdopen(int, const char *);
FILE *popen(const char *, const char *);
int pclose(FILE *);
int fileno(FILE *);
int fseeko(FILE *, off_t, int);
off_t ftello(FILE *);
int dprintf(int, const char *restrict, ...);
int vdprintf(int, const char *restrict, __isoc_va_list);
void flockfile(FILE *);
int ftrylockfile(FILE *);
void funlockfile(FILE *);
int getc_unlocked(FILE *);
int getchar_unlocked(void);
int putc_unlocked(int, FILE *);
int putchar_unlocked(int);
ssize_t getdelim(char **restrict, size_t *restrict, int, FILE *restrict);
ssize_t getline(char **restrict, size_t *restrict, FILE *restrict);
int renameat(int, const char *, int, const char *);
char *ctermid(char *);
# 10 "include/shared/util/log.h" 2



# 12 "include/shared/util/log.h"
enum log_level {
	ll_quiet,
	ll_warn,
	ll_info,
	ll_debug,
	log_level_count,
};

extern FILE *logfile;
extern enum log_level log_level;
extern
# 22 "include/shared/util/log.h" 3 4
_Bool
# 22 "include/shared/util/log.h"
logging_initialized;
# 46 "include/shared/util/log.h"
void log_bytes(const void *src, size_t size);
void set_log_file(const char *otparg);
void set_log_lvl(const char *otparg);
void log_bytes_r(const void *src, size_t size);
void log_init(void);
# 15 "client/cfg/keymap.c" 2

enum tables {
	table_keycmd,
	table_im,
	table_constants
};

static struct cfg_lookup_table ltbl[] = {
	[table_keycmd] = {
		"none", kc_none,
		"invalid", kc_invalid,
		"center", kc_center,
		"center_cursor", kc_center_cursor,
		"view_left", kc_view_left,
		"view_down", kc_view_down,
		"view_up", kc_view_up,
		"view_right", kc_view_right,
		"find", kc_find,
		"set_input_mode", kc_set_input_mode,
		"quit", kc_quit,
		"cursor_left", kc_cursor_left,
		"cursor_down", kc_cursor_down,
		"cursor_up", kc_cursor_up,
		"cursor_right", kc_cursor_right,
		"set_action_type", kc_set_action_type,
		"set_action_target", kc_set_action_target,
		"toggle_action_flag", kc_toggle_action_flag,
		"read_action_target", kc_read_action_target,
		"swap_cursor_with_source", kc_swap_cursor_with_source,
		"set_action_height", kc_set_action_height,
		"action_height_grow", kc_action_height_grow,
		"action_height_shrink", kc_action_height_shrink,
		"set_action_width", kc_set_action_width,
		"action_width_grow", kc_action_width_grow,
		"action_width_shrink", kc_action_width_shrink,
		"action_rect_rotate", kc_action_rect_rotate,
		"undo_action", kc_undo_action,
		"exec_action", kc_exec_action,
		"toggle_help", kc_toggle_help,
		"", kc_macro,
	},
	[table_im] = {
		"normal", im_normal,
		"select", im_select,
		"resize", im_resize,
		"cmd", im_cmd,
	},
	[table_constants] = {
		"tile_deep_water", tile_deep_water,
		"tile_water", tile_water,
		"tile_wetland", tile_wetland,
		"tile_plain", tile_plain,
		"tile_forest", tile_forest,
		"tile_mountain", tile_mountain,
		"tile_peak", tile_peak,
		"tile_dirt", tile_dirt,
		"tile_forest_young", tile_forest_young,
		"tile_forest_old", tile_forest_old,
		"tile_wetland_forest_young", tile_wetland_forest_young,
		"tile_wetland_forest", tile_wetland_forest,
		"tile_wetland_forest_old", tile_wetland_forest_old,
		"tile_coral", tile_coral,
		"tile_stream", tile_stream,
		"tile_wood", tile_wood,
		"tile_stone", tile_stone,
		"tile_wood_floor", tile_wood_floor,
		"tile_rock_floor", tile_rock_floor,
		"tile_storehouse", tile_storehouse,
		"tile_farmland_empty", tile_farmland_empty,
		"tile_farmland_done", tile_farmland_done,
		"tile_burning", tile_burning,
		"tile_burnt", tile_burnt,
		"at_none", at_none,
		"at_move", at_move,
		"at_harvest", at_harvest,
		"at_build", at_build,
		"at_fight", at_fight,
		"at_carry", at_carry,
		"im_select", im_select,
		"im_normal", im_normal,
		"im_resize", im_resize,
		"im_cmd", im_cmd,
	},
};

static int
next_key(const char **str)
{
	int k;

	switch (k = *(++*str)) {
	case '\\':
		switch (k = *(++*str)) {
		case 'u':
			return skc_up;
		case 'd':
			return skc_down;
		case 'l':
			return skc_left;
		case 'r':
			return skc_right;
		case 'n':
			return '\n';
		case 't':
			return '\t';
		case 's':
			return ' ';
		default:
			return k;
		case '\0':
			return -1;
		}
	case '\0':
		return -1;
	default:
		return k;
	}
}


static
# 135 "client/cfg/keymap.c" 3 4
_Bool

# 136 "client/cfg/keymap.c"
parse_macro(char *err, char *buf, const char *macro){
	uint32_t i, bufi = 0, const_bufi, mode = 0;
	int32_t constant;
	char const_buf[32 + 1] = { 0 };

	for (i = 0; macro[i] != '\0'; ++i) {
		if (macro[i] == '\0') {
			break;
		} else if (mode == 1) {
			if (macro[i] == '>') {
				const_buf[const_bufi] = 0;

				if ((constant = cfg_string_lookup(const_buf,
					&ltbl[table_constants])) == -1) {
					if (log_level >= ll_debug) {
						do {
							((void)((
									logging_initialized
									) || (__assert_fail(
									"logging_initialized"
									, "client/cfg/keymap.c", 151, __func__), 0)))
							; if (logfile ==
							      (stderr)
							      ) {
								fprintf(logfile, "[\033[%dm" "debug" "\033[0m] %s:%d [\033[35m%s\033[0m] ", 0, "client/cfg/keymap.c", 151, __func__);
							} else {
								fprintf(logfile, "[" "debug" "] %s:%d [%s] ", "client/cfg/keymap.c", 151, __func__);
							}
						} while (0); do {
							fprintf(logfile, "invalid constant name: '%s' while parsing macro: '%s'", const_buf, macro); fprintf(logfile, "\n");
						} while (0);
					}
					; snprintf(err, 256, "invalid constant name: '%s' while parsing macro: '%s'", const_buf, macro)
					;
					return
# 153 "client/cfg/keymap.c" 3 4
						0
# 153 "client/cfg/keymap.c"
					;
				}

				bufi += snprintf(&buf[bufi], 32 - bufi, "%d", constant);

				mode = 0;
			} else {
				const_buf[const_bufi++] = macro[i];

				if (const_bufi >= 32) {
					if (log_level >= ll_debug) {
						do {
# 163 "client/cfg/keymap.c" 3 4
							((void)((
# 163 "client/cfg/keymap.c"
									logging_initialized
# 163 "client/cfg/keymap.c" 3 4
									) || (__assert_fail(
# 163 "client/cfg/keymap.c"
									"logging_initialized"
# 163 "client/cfg/keymap.c" 3 4
									, "client/cfg/keymap.c", 163, __func__), 0)))
# 163 "client/cfg/keymap.c"
							; if (logfile ==
# 163 "client/cfg/keymap.c" 3 4
							      (stderr)
# 163 "client/cfg/keymap.c"
							      ) {
								fprintf(logfile, "[\033[%dm" "debug" "\033[0m] %s:%d [\033[35m%s\033[0m] ", 0, "client/cfg/keymap.c", 163, __func__);
							} else {
								fprintf(logfile, "[" "debug" "] %s:%d [%s] ", "client/cfg/keymap.c", 163, __func__);
							}
						} while (0); do {
							fprintf(logfile, "const too long while parsing macro: '%s'", macro); fprintf(logfile, "\n");
						} while (0);
					}
					; snprintf(err, 256, "const too long while parsing macro: '%s'", macro);
					return
# 164 "client/cfg/keymap.c" 3 4
						0
# 164 "client/cfg/keymap.c"
					;
				}
			}

		} else if (macro[i] == '<') {
			const_bufi = 0;
			mode = 1;
		} else {
			buf[bufi++] = macro[i];
		}

		if (bufi >= 32) {
			if (log_level >= ll_debug) {
				do {
# 176 "client/cfg/keymap.c" 3 4
					((void)((
# 176 "client/cfg/keymap.c"
							logging_initialized
# 176 "client/cfg/keymap.c" 3 4
							) || (__assert_fail(
# 176 "client/cfg/keymap.c"
							"logging_initialized"
# 176 "client/cfg/keymap.c" 3 4
							, "client/cfg/keymap.c", 176, __func__), 0)))
# 176 "client/cfg/keymap.c"
					; if (logfile ==
# 176 "client/cfg/keymap.c" 3 4
					      (stderr)
# 176 "client/cfg/keymap.c"
					      ) {
						fprintf(logfile, "[\033[%dm" "debug" "\033[0m] %s:%d [\033[35m%s\033[0m] ", 0, "client/cfg/keymap.c", 176, __func__);
					} else {
						fprintf(logfile, "[" "debug" "] %s:%d [%s] ", "client/cfg/keymap.c", 176, __func__);
					}
				} while (0); do {
					fprintf(logfile, "macro '%s' too long", macro); fprintf(logfile, "\n");
				} while (0);
			}
			; snprintf(err, 256, "macro '%s' too long", macro);
			return
# 177 "client/cfg/keymap.c" 3 4
				0
# 177 "client/cfg/keymap.c"
			;
		}
	}

	if (mode == 1) {
		if (log_level >= ll_debug) {
			do {
# 182 "client/cfg/keymap.c" 3 4
				((void)((
# 182 "client/cfg/keymap.c"
						logging_initialized
# 182 "client/cfg/keymap.c" 3 4
						) || (__assert_fail(
# 182 "client/cfg/keymap.c"
						"logging_initialized"
# 182 "client/cfg/keymap.c" 3 4
						, "client/cfg/keymap.c", 182, __func__), 0)))
# 182 "client/cfg/keymap.c"
				; if (logfile ==
# 182 "client/cfg/keymap.c" 3 4
				      (stderr)
# 182 "client/cfg/keymap.c"
				      ) {
					fprintf(logfile, "[\033[%dm" "debug" "\033[0m] %s:%d [\033[35m%s\033[0m] ", 0, "client/cfg/keymap.c", 182, __func__);
				} else {
					fprintf(logfile, "[" "debug" "] %s:%d [%s] ", "client/cfg/keymap.c", 182, __func__);
				}
			} while (0); do {
				fprintf(logfile, "missing '>' while parsing macro: '%s'", macro); fprintf(logfile, "\n");
			} while (0);
		}
		; snprintf(err, 256, "missing '>' while parsing macro: '%s'", macro);
		return
# 183 "client/cfg/keymap.c" 3 4
			0
# 183 "client/cfg/keymap.c"
		;
	}

	return
# 186 "client/cfg/keymap.c" 3 4
		1
# 186 "client/cfg/keymap.c"
	;
}

static
# 189 "client/cfg/keymap.c" 3 4
_Bool

# 190 "client/cfg/keymap.c"
set_keymap(struct keymap *km, char *err, const char *c, const char *v, enum key_command kc){
	int tk, nk;
	const char **cp = &c;
	uint8_t trigger_i = 0;
	char trigger_buf[32] = { 0 };

	(*cp)--;

	if ((tk = next_key(cp)) == -1) {
		if (log_level >= ll_debug) {
			do {
# 200 "client/cfg/keymap.c" 3 4
				((void)((
# 200 "client/cfg/keymap.c"
						logging_initialized
# 200 "client/cfg/keymap.c" 3 4
						) || (__assert_fail(
# 200 "client/cfg/keymap.c"
						"logging_initialized"
# 200 "client/cfg/keymap.c" 3 4
						, "client/cfg/keymap.c", 200, __func__), 0)))
# 200 "client/cfg/keymap.c"
				; if (logfile ==
# 200 "client/cfg/keymap.c" 3 4
				      (stderr)
# 200 "client/cfg/keymap.c"
				      ) {
					fprintf(logfile, "[\033[%dm" "debug" "\033[0m] %s:%d [\033[35m%s\033[0m] ", 0, "client/cfg/keymap.c", 200, __func__);
				} else {
					fprintf(logfile, "[" "debug" "] %s:%d [%s] ", "client/cfg/keymap.c", 200, __func__);
				}
			} while (0); do {
				fprintf(logfile, "key is empty"); fprintf(logfile, "\n");
			} while (0);
		}
		; snprintf(err, 256, "key is empty");
		return
# 201 "client/cfg/keymap.c" 3 4
			0
# 201 "client/cfg/keymap.c"
		;
	}

	while ((nk = next_key(cp)) != -1) {
		if (tk > 128) {
			if (log_level >= ll_debug) {
				do {
# 206 "client/cfg/keymap.c" 3 4
					((void)((
# 206 "client/cfg/keymap.c"
							logging_initialized
# 206 "client/cfg/keymap.c" 3 4
							) || (__assert_fail(
# 206 "client/cfg/keymap.c"
							"logging_initialized"
# 206 "client/cfg/keymap.c" 3 4
							, "client/cfg/keymap.c", 206, __func__), 0)))
# 206 "client/cfg/keymap.c"
					; if (logfile ==
# 206 "client/cfg/keymap.c" 3 4
					      (stderr)
# 206 "client/cfg/keymap.c"
					      ) {
						fprintf(logfile, "[\033[%dm" "debug" "\033[0m] %s:%d [\033[35m%s\033[0m] ", 0, "client/cfg/keymap.c", 206, __func__);
					} else {
						fprintf(logfile, "[" "debug" "] %s:%d [%s] ", "client/cfg/keymap.c", 206, __func__);
					}
				} while (0); do {
					fprintf(logfile, "'%c' (%d) is outside of ascii range", tk, tk); fprintf(logfile, "\n");
				} while (0);
			}
			; snprintf(err, 256, "'%c' (%d) is outside of ascii range", tk, tk);
			return
# 207 "client/cfg/keymap.c" 3 4
				0
# 207 "client/cfg/keymap.c"
			;
		}

		trigger_buf[trigger_i++] = tk;
		if (trigger_i >= 32 - 1) {
			if (log_level >= ll_debug) {
				do {
# 212 "client/cfg/keymap.c" 3 4
					((void)((
# 212 "client/cfg/keymap.c"
							logging_initialized
# 212 "client/cfg/keymap.c" 3 4
							) || (__assert_fail(
# 212 "client/cfg/keymap.c"
							"logging_initialized"
# 212 "client/cfg/keymap.c" 3 4
							, "client/cfg/keymap.c", 212, __func__), 0)))
# 212 "client/cfg/keymap.c"
					; if (logfile ==
# 212 "client/cfg/keymap.c" 3 4
					      (stderr)
# 212 "client/cfg/keymap.c"
					      ) {
						fprintf(logfile, "[\033[%dm" "debug" "\033[0m] %s:%d [\033[35m%s\033[0m] ", 0, "client/cfg/keymap.c", 212, __func__);
					} else {
						fprintf(logfile, "[" "debug" "] %s:%d [%s] ", "client/cfg/keymap.c", 212, __func__);
					}
				} while (0); do {
					fprintf(logfile, "trigger too long"); fprintf(logfile, "\n");
				} while (0);
			}
			; snprintf(err, 256, "trigger too long");
			return
# 213 "client/cfg/keymap.c" 3 4
				0
# 213 "client/cfg/keymap.c"
			;
		}

		km = &km->map[tk];

		if (km->map ==
# 218 "client/cfg/keymap.c" 3 4
		    ((void*)0)
# 218 "client/cfg/keymap.c"
		    ) {
			keymap_init(km);
		}

		tk = nk;
	}

	trigger_buf[trigger_i++] = tk;
	km->map[tk].cmd = kc;

	if (kc == kc_macro) {
		if (!parse_macro(km->map[tk].strcmd, err, v)) {
			return
# 230 "client/cfg/keymap.c" 3 4
				0
# 230 "client/cfg/keymap.c"
			;
		}
	}

	strncpy(km->map[tk].trigger, trigger_buf, 32);


	return
# 237 "client/cfg/keymap.c" 3 4
		1
# 237 "client/cfg/keymap.c"
	;
}

struct parse_keymap_ctx {
	struct keymap *km;
	struct ui_ctx *ui_ctx;
};

static
# 245 "client/cfg/keymap.c" 3 4
_Bool

# 246 "client/cfg/keymap.c"
parse_keymap_handler(void *_ctx, char *err, const char *sec, const char *k, const char *v, uint32_t line){
	struct parse_keymap_ctx *ctx = _ctx;
	struct keymap *km = ctx->km;
	int32_t im, kc;

	if (sec ==
# 252 "client/cfg/keymap.c" 3 4
	    ((void*)0)
# 252 "client/cfg/keymap.c"
	    ) {
		if (log_level >= ll_debug) {
			do {
# 253 "client/cfg/keymap.c" 3 4
				((void)((
# 253 "client/cfg/keymap.c"
						logging_initialized
# 253 "client/cfg/keymap.c" 3 4
						) || (__assert_fail(
# 253 "client/cfg/keymap.c"
						"logging_initialized"
# 253 "client/cfg/keymap.c" 3 4
						, "client/cfg/keymap.c", 253, __func__), 0)))
# 253 "client/cfg/keymap.c"
				; if (logfile ==
# 253 "client/cfg/keymap.c" 3 4
				      (stderr)
# 253 "client/cfg/keymap.c"
				      ) {
					fprintf(logfile, "[\033[%dm" "debug" "\033[0m] %s:%d [\033[35m%s\033[0m] ", 0, "client/cfg/keymap.c", 253, __func__);
				} else {
					fprintf(logfile, "[" "debug" "] %s:%d [%s] ", "client/cfg/keymap.c", 253, __func__);
				}
			} while (0); do {
				fprintf(logfile, "mapping in without section not allowed. While parsing keymap at line %d", line); fprintf(logfile, "\n");
			} while (0);
		}
		; snprintf(err, 256, "mapping in without section not allowed. While parsing keymap at line %d", line);
		return
# 254 "client/cfg/keymap.c" 3 4
			0
# 254 "client/cfg/keymap.c"
		;
	}

	switch (ui_keymap_hook(ctx->ui_ctx, ctx->km, sec, k, v, line)) {
	case khr_matched:
		return
# 259 "client/cfg/keymap.c" 3 4
			1
# 259 "client/cfg/keymap.c"
		;
	case khr_unmatched:
		break;
	case khr_failed:
		return
# 263 "client/cfg/keymap.c" 3 4
			0
# 263 "client/cfg/keymap.c"
		;
	}


	if ((im = cfg_string_lookup(sec, &ltbl[table_im])) == -1) {
		if (log_level >= ll_info) {
			do {
# 268 "client/cfg/keymap.c" 3 4
				((void)((
# 268 "client/cfg/keymap.c"
						logging_initialized
# 268 "client/cfg/keymap.c" 3 4
						) || (__assert_fail(
# 268 "client/cfg/keymap.c"
						"logging_initialized"
# 268 "client/cfg/keymap.c" 3 4
						, "client/cfg/keymap.c", 268, __func__), 0)))
# 268 "client/cfg/keymap.c"
				; if (logfile ==
# 268 "client/cfg/keymap.c" 3 4
				      (stderr)
# 268 "client/cfg/keymap.c"
				      ) {
					fprintf(logfile, "[\033[%dm" "info" "\033[0m] %s:%d [\033[35m%s\033[0m] ", 34, "client/cfg/keymap.c", 268, __func__);
				} else {
					fprintf(logfile, "[" "info" "] %s:%d [%s] ", "client/cfg/keymap.c", 268, __func__);
				}
			} while (0); do {
				fprintf(logfile, "skipping unmatched section '%s' line %d", sec, line); fprintf(logfile, "\n");
			} while (0);
		}
		;
		return
# 269 "client/cfg/keymap.c" 3 4
			1
# 269 "client/cfg/keymap.c"
		;
	}


# 272 "client/cfg/keymap.c" 3 4
	((void)((
# 272 "client/cfg/keymap.c"
			k !=
# 272 "client/cfg/keymap.c" 3 4
			((void*)0)) || (__assert_fail(
# 272 "client/cfg/keymap.c"
			"k != NULL"
# 272 "client/cfg/keymap.c" 3 4
			, "client/cfg/keymap.c", 272, __func__), 0)))
# 272 "client/cfg/keymap.c"
	;

# 273 "client/cfg/keymap.c" 3 4
	((void)((
# 273 "client/cfg/keymap.c"
			v !=
# 273 "client/cfg/keymap.c" 3 4
			((void*)0)) || (__assert_fail(
# 273 "client/cfg/keymap.c"
			"v != NULL"
# 273 "client/cfg/keymap.c" 3 4
			, "client/cfg/keymap.c", 273, __func__), 0)))
# 273 "client/cfg/keymap.c"
	;

	if ((kc = cfg_string_lookup(v, &ltbl[table_keycmd])) == -1) {
		kc = kc_macro;
	}

	if (!(set_keymap(&km[im], err, k, v, kc))) {
		return
# 280 "client/cfg/keymap.c" 3 4
			0
# 280 "client/cfg/keymap.c"
		;
	}

	return
# 283 "client/cfg/keymap.c" 3 4
		1
# 283 "client/cfg/keymap.c"
	;
}


# 286 "client/cfg/keymap.c" 3 4
_Bool

# 287 "client/cfg/keymap.c"
parse_keymap(struct keymap *km, struct ui_ctx *ui_ctx){
	struct parse_keymap_ctx ctx = { .km = km, .ui_ctx = ui_ctx };

	return parse_cfg_file("keymap.ini", &ctx, parse_keymap_handler);
}
