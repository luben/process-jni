package com.github.luben.process;

public class Process {

    static {
        Native.load();
    }
    /**
     * Fork new process.
     * @return  The pid or 0
     */
    public static native int fork();
    public static native int waitpid(int pid, int options);
    public static native int execv(String path, String[] args);
    public static native int posix_spawn(String path, String[] args);
    public static native int prctl(int option, long arg2, long arg3, long arg4, long arg5);
    public static native String[] getgrouplist(String user);
}
