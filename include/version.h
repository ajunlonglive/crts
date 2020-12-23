#ifndef VERSION_H
#define VERSION_H
struct crts_version {
	const char *version, *vcs_tag;
};

extern const struct crts_version crts_version;
#endif
