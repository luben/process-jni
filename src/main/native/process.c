#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#ifdef IS_LINUX
#include <sys/prctl.h>
#endif
#include <spawn.h>
#include <grp.h>
#include <pwd.h>
#include <jni.h>

extern char **environ;

static char *process_args = NULL;
static int process_args_size = 0;

static void throw(JNIEnv *jenv, char *className, char *message) {
    jclass klass = (*jenv)->FindClass(jenv, className);
    (*jenv)->ThrowNew(jenv, klass, message);
}

static void outOfMemory(JNIEnv *env) {
    throw(env, "java/lang/OutOfMemoryError",
        "cannot allocate more memory in native call");
}

static void find_args(void) {
    if (process_args != NULL) return;
    char *p = environ[0] - 1;
    size_t size = 0;
    while(1) {
        while(*p) {
            p--;
            size++;
        }
        p--;
        size++;
        if (!*p) break;
    }
    p+=2;
    size--;
    process_args = p;
    process_args_size = size;
}

static char *const *fetchArgv(JNIEnv *jenv, jobjectArray jargv) {
    jsize argc = (*jenv)->GetArrayLength(jenv, jargv);
    const char **argv = (const char**)calloc(sizeof(char*), argc+1);
    if(!argv) {
        outOfMemory(jenv);
        return NULL;
    }
    int i = 0;
    for(i=0; i<argc; i++) {
        jobject jtext = (*jenv)->GetObjectArrayElement(jenv, jargv, i);
        const char *utf8text = (*jenv)->GetStringUTFChars(jenv, jtext, NULL);
        argv[i] = utf8text;
    }
    argv[i] = NULL;
    return (char *const *) argv;
}

static void freeArgv(JNIEnv *jenv, char *const *argv, jobjectArray jargv) {
    int i = 0;
    char *const *p;
    for(p=argv, i=0; *p; i++, p++) {
        jobject jtext = (*jenv)->GetObjectArrayElement(jenv, jargv, i);
        (*jenv)->ReleaseStringUTFChars(jenv, jtext, *p);
    }
    free((void *)argv);
}

/*
 * Class:     com_github_luben_process_Process
 * Method:    setName
 * Signature: (Ljava/lang/String)
 */
JNIEXPORT void JNICALL Java_com_github_luben_process_Process_setName
  (JNIEnv *jenv, jclass klass, jstring jName) {
#ifdef IS_LINUX
    const char *name  = (*jenv)->GetStringUTFChars(jenv, jName, NULL);
    find_args();
    memset(process_args, 0, process_args_size);
    snprintf(process_args, process_args_size, "%s", name);
#endif
}

/*
 * Class:     com_github_luben_process_Process
 * Method:    getNameLimit
 * Signature: (Ljava/lang/String)I
 */
JNIEXPORT jint JNICALL Java_com_github_luben_process_Process_getNameLimit
  (JNIEnv *jenv, jclass klass) {
#ifdef IS_LINUX
    find_args();
#endif
    return process_args_size;
}

JNIEXPORT jint JNICALL Java_com_github_luben_process_Process_fork
  (JNIEnv *jenv, jclass klass) {
    return fork();
}

/*
 * Class:     com_github_luben_process_Process
 * Method:    waitpid
 * Signature: (IZ)I
 */
JNIEXPORT jint JNICALL Java_com_github_luben_process_Process_waitpid
  (JNIEnv *jenv, jclass klass, jint pid, jint options) {
    int status = 0;
    waitpid(pid, &status, options);
    return status;
}

/*
 * Class:     com_github_luben_process_Process
 * Method:    execv
 * Signature: (Ljava/lang/String;[Ljava/lang/String;)I
 */
JNIEXPORT jint JNICALL Java_com_github_luben_process_Process_execv
  (JNIEnv *jenv, jclass klass, jstring jpath, jobjectArray params) {

    const char *path  = (*jenv)->GetStringUTFChars(jenv, jpath, NULL);
    char *const *args = fetchArgv(jenv, params);

    execv(path, args);
    return 0;
}

/*
 * Class:     com_github_luben_process_Process
 * Method:    spawn
 * Signature: (Ljava/lang/String;[Ljava/lang/String;)I
 */
JNIEXPORT jint JNICALL Java_com_github_luben_process_Process_posix_1spawn
  (JNIEnv *jenv, jclass klass, jstring jpath, jobjectArray params) {
    const char *path  = (*jenv)->GetStringUTFChars(jenv, jpath, NULL);
    char *const *args = fetchArgv(jenv, params);
    int pid;
    int status = posix_spawn(&pid, path, NULL, NULL, args, environ);
    if (status != 0) {
        throw(jenv, "java/lang/RuntimeException", strerror(status));
        return 0;
    }
    freeArgv(jenv, args, params);
    return pid;
}

/*
 * Class:     com_github_luben_process_Process
 * Method:    prctl
 * Signature: (IJJJJJ)I
 */
JNIEXPORT jint JNICALL Java_com_github_luben_process_Process_prctl
  (JNIEnv *jenv, jclass klass, jint option, jlong arg2, jlong arg3, jlong arg4, jlong arg5) {
#ifdef IS_LINUX
    return prctl(option, arg2, arg3, arg4, arg5);
#endif
    return -1;
}


/*
 * Class:     com_github_luben_process_Process
 * Method:    getgrouplist
 * Signature: (Ljava/lang/String;)[Ljava/lang/String;
 */
JNIEXPORT jobjectArray JNICALL Java_com_github_luben_process_Process_getgrouplist
  (JNIEnv *jenv, jclass klass, jstring juser) {

    const char *user  = (*jenv)->GetStringUTFChars(jenv, juser, NULL);
    int ngroups = 20;
    int i;
    gid_t *groups = malloc(ngroups * sizeof(gid_t));
    struct group  *gr;
    struct passwd *pw;

    jobjectArray result;

    pw = getpwnam(user);

    if (pw == NULL) {
        free(groups);
        throw(jenv, "java/lang/RuntimeException", "getpwnam");
        return NULL;
    }

    if (getgrouplist(user, pw->pw_gid, groups, &ngroups) == -1) {
        free(groups);
        groups = malloc(ngroups * sizeof(gid_t));
        if (getgrouplist(user, pw->pw_gid, groups, &ngroups) == -1) {
            free(groups);
            throw(jenv, "java/lang/RuntimeException", "getpwnam");
            return NULL;
        }
    }

    result = (jobjectArray)(*jenv)->NewObjectArray(jenv,
            (jsize) ngroups,
            (*jenv)->FindClass(jenv, "java/lang/String"),
            (*jenv)->NewStringUTF(jenv, ""));
    if (result == NULL) {
        free(groups);
        outOfMemory(jenv);
        return NULL;
    }

    for (i = 0; i < ngroups; i++) {
        gr = getgrgid(groups[i]);
        if (gr != NULL) {
            (*jenv)->SetObjectArrayElement(
                    jenv, result, (jsize) i,
                    (*jenv)->NewStringUTF(jenv, gr->gr_name));
        }
    }

    free(groups);
    return(result);
}
