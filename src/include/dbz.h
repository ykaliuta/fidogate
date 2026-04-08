/* for dbm and dbz */
typedef struct {
    char *dptr;
    int dsize;
} datum;

/* standard dbm functions */
extern int dbminit(char *name);
extern datum fetch(datum key);
extern int store(datum key, datum data);
extern int delete(datum key);           /* not in dbz */
extern datum firstkey(void);            /* not in dbz */
extern datum nextkey(datum key);        /* not in dbz */
extern int dbmclose(void);              /* in dbz, but not in old dbm */

/* new stuff for dbz */
extern int dbzfresh(char *name, long size, int fs, int cmap, long tagmask);
extern int dbzagain(char *name, char *oldname);
extern datum dbzfetch(datum key);
extern datum dbcfetch(datum key);
extern int dbzstore(datum key, datum data);
extern int dbzsync(void);
extern long dbzsize(long contents);
extern int dbzincore(int value);
extern int dbzcancel(void);
extern int dbzdebug(int value);

/*
 * In principle we could handle unlimited-length keys by operating a chunk
 * at a time, but it's not worth it in practice.  Setting a nice large
 * bound on them simplifies the code and doesn't hurt anything.
 */
#define DBZMAXKEY	255
