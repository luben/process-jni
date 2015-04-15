#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/prctl.h>
#include <spawn.h>
#include <jni.h>

extern char **environ;

static void throw(JNIEnv *jenv, char *className, char *message) {
    jclass klass = (*jenv)->FindClass(jenv, className);
    (*jenv)->ThrowNew(jenv, klass, message);
}

static void outOfMemory(JNIEnv *env) {
    throw(env, "java/lang/OutOfMemoryError",
        "cannot allocate more memory in native call");
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
        throw(jenv, "java/lang/RunitimeException", strerror(status));
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
    return prctl(option, arg2, arg3, arg4, arg5);
}
