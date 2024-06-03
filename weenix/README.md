# weenix
**Final Project for CSCI1690: Operating Systems**

A full operating system kernel, based on Linux, built as a semester-long project in [CSCI 1670/1690 (Operating Systems)](https://brown-cs1690.github.io/brown-cs167-s24/) at Brown University.

The projects, in order of completion:

* **Procs** - Threads, processes, scheduling and synchronization primitives.
* **Drivers** - Device drivers for terminals, disks, and the memory devices /dev/zero and /dev/null.
* **VFS (Virtual File System)** - A polymorphic interface between the operating system kernel and the various file systems (such as S5FS and device drivers).
* **S5FS (System V File System)** - A file system implementation based on the original Unix file system.
* **VM (Virtual Memory)** - Userspace address space management, running user-level code, servicing system calls, and basically everything else needed to combine all of the previous components into a fully functioning operating system. This includes virtual memory maps, handling page faults, memory management via anonymous objects and shadow objects, and system calls (in particular, the fork syscall).

NOTE: This is a placeholder repo with no code to respect Brown's Academic Code. If you are a potential employer and would like to look at the code, please send me an email at melvin_he@brown.edu.