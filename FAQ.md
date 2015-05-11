# Introduction #

Hardlink is UNICODE rebuild of [Sergey Oboguev's hardlink](http://photo.net/digital-darkroom-forum/00Etbl). All my changes were to add UNICODE awareness to this useful program.


# Details #
_Sergey Oboguev's_ description:

Some time ago I was looking for a solution to split large image file sets for burning onto multiple DVDs. As I was unable to find satisfactory existing solution for my purposes, I put together my own utility to do this. Some inquired me if I have found a solution, so those who have similar needs may use it.

This utitily works only on Windows 2000/XP and only on NTFS volumes. It uses NTFS feature called hard links.

Directory entry on NTFS volume is not a file itself but a pointer to a file stream. Each file can be linked from multiple directories. Actual file gets deleted when the last link to it is gone.

Utility I wrote has two parts:

  1. Part one allows to copy any source directory tree to another target directory as hard links. Files are not duplicated in the process. Created directory tree contains references to original files, not a copy of the files. Thus very little extra disk space is consumed in the process. Created directories and subdirectories however are separate from the original tree, only files are linked, not original directories. Thus if you copy directory tree SRC to directory tree DST and perform manipulations on DST, this does not affect SRC layout. You may delete files from DST folders, this won't affect SRC folders. After you are done with DST, you can delete it altogether, this won't delete the files (assuming you have not deleted them in SRC as well).

  1. Part two allows to split directory tree into multiple "disks" (folders named disk1, disk2 and so on) each having maximum specified size. You can select one of pre-set sizes from the combo box control or type in your own size. Those "disk" folders can be subsequently burned to DVDs or CDs. Once you are done with burning, you can delete "disk" folders and/or their root folder and their contents. Again, this won't affect source files. Splitter takes care to keep reasonable data sequencing across disks.

Part 1 is needed as one may want to avoid burning some of the files in SRC tree since they may have already been burned. In this case, instead of splittig SRC to DST directly, one may copy SRC to TMP as hard links, delete unneeded files from TMP and then split TMP into DST, then delete TMP and burn "disk" folders from DST.

I included executable file and source code, but I did not include dependent DLLs (MFC70, MSVCRT, MSVCR70, COMCTL32).

Enjoy.