/* connector for stat */

#include <reent.h>
#include <sys/stat.h>
#include <unistd.h>

int
_DEFUN (stat, (file, pstat),
     _CONST char *file _AND
     struct stat *pstat)
{
  return _stat_r (_REENT, file, pstat);
}
