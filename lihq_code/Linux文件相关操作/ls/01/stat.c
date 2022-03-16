#include "stat.h"

void listl(char *path)
{
  struct stat pstat;
  glob_t pglob;
  char buf[BUFFSIZE] = {};
  if (lstat(path, &pstat) < 0)
  {
    perror("lstat()");
    return;
  }
  if (!S_ISDIR(pstat.st_mode))
  {
    statjudge(path);
    return;
  }
  strcpy(buf, path);
  strcat(buf, "/*");
  glob(buf, 0, NULL, &pglob);
  //  memset(buf,'\0',BUFFSIZE);
  for (int i = 0; i < pglob.gl_pathc; i++)
  {
    statjudge((pglob.gl_pathv)[i]);
  }
  globfree(&pglob);
}

void listi(char *path)
{
  struct stat pstat;
  glob_t pglob;
  char buf[BUFFSIZE] = {};
  if (lstat(path, &pstat) < 0)
  {
    perror("lstat()");
    return;
  }
  if (!S_ISDIR(pstat.st_mode))
  {
    printf("%ld  %s\n", pstat.st_ino, path);
    return;
  }
  strcpy(buf, path);
  strcat(buf, "/*");
  glob(buf, 0, NULL, &pglob);
  //  memset(buf,'\0',BUFFSIZE);
  for (int i = 0; i < pglob.gl_pathc; i++)
  {
    if (lstat(((pglob.gl_pathv)[i]), &pstat) < 0)
    {
      perror("lstat()");
      return;
    }
    char *o = strrchr((pglob.gl_pathv)[i], '/');
    printf("%ld  %s\n", pstat.st_ino, ++o);
  }
  globfree(&pglob);
}

void lista(char *path)
{
  struct stat pstat;
  glob_t pglob;
  char buf[BUFFSIZE] = {};
  if (lstat(path, &pstat) < 0)
  {
    perror("lstat()");
    return;
  }
  if (!S_ISDIR(pstat.st_mode))
  {
    printf("%s  ", path);
    return;
  }
  DIR *dp = opendir(path);
  struct dirent *rd = NULL;
  while (1)
  {
    rd = readdir(dp);
    if (rd == NULL)
    {
      if (errno)
      {
        perror("readdir()");
        closedir(dp);
        return;
      }
      break;
    }
    printf("%s  ", rd->d_name);
  }
  closedir(dp);
  printf("\n");
  return;
}

void listh(char *path)
{
  struct stat pstat;
  //  char *p=NULL;
  glob_t pglob;
  char buf[BUFFSIZE] = {};

  if (lstat(path, &pstat) < 0)
  {
    perror("lstat()");
    return;
  }
  if (!S_ISDIR(pstat.st_mode))
  {
    printf("%s  ", path);
    return;
  }
  DIR *dp = opendir(path);
  struct dirent *rd = NULL;
  while (1)
  {
    rd = readdir(dp);
    if (rd == NULL)
    {
      if (errno)
      {
        perror("readdir()");
        closedir(dp);
        return;
      }
      break;
    }
    if ((rd->d_name)[0] == '.')
      continue;
    printf("%s  ", rd->d_name);
  }
  closedir(dp);
  printf("\n");
}

void statjudge(char *path)
{

  struct stat mystat;
  if (stat(path, &mystat) == -1)
  {
    perror("stat()");
    return;
  }
  switch (mystat.st_mode & S_IFMT)
  {
  case S_IFSOCK:
    printf("s");
    break;
  case S_IFLNK:
    printf("l");
    break;
  case S_IFREG:
    printf("-");
    break;
  case S_IFBLK:
    printf("b");
    break;
  case S_IFDIR:
    printf("d");
    break;
  case S_IFCHR:
    printf("c");
    break;
  case S_IFIFO:
    printf("p");
    break;
  fefault:
    break;
  }
  //  printf("\n");
  if (mystat.st_mode & S_IRUSR)
    putchar('r');
  else
    putchar('-');
  if (mystat.st_mode & S_IWUSR)
    putchar('w');
  else
    putchar('-');
  if (mystat.st_mode & S_IXUSR)
    if (mystat.st_mode & S_ISUID)
      putchar('s');
    else
      putchar('x');
  else
    putchar('-');
  if (mystat.st_mode & S_IRGRP)
    putchar('r');
  else
    putchar('-');
  if (mystat.st_mode & S_IWGRP)
    putchar('w');
  else
    putchar('-');
  if (mystat.st_mode & S_IXGRP)
    if (mystat.st_mode & S_ISGID)
      putchar('s');
    else
      putchar('x');
  else
    putchar('-');
  if (mystat.st_mode & S_IROTH)
    putchar('r');
  else
    putchar('-');
  if (mystat.st_mode & S_IWOTH)
    putchar('w');
  else
    putchar('-');
  if (mystat.st_mode & S_IXOTH)
    if (mystat.st_mode & S_ISVTX)
      putchar('t');
    else
      putchar('x');
  else
    putchar('-');

  printf(" %ld", mystat.st_nlink);

  struct passwd *pwd = NULL; //文件拥有者
  pwd = getpwuid(mystat.st_uid);
  printf(" %s ", pwd->pw_name);

  struct group *grp = NULL; //文件所属组
  grp = getgrgid(mystat.st_gid);
  printf(" %s ", grp->gr_name);

  printf(" %ld ", mystat.st_size); //总字节个数

  //获取文件时间　atime mtime ctime
  struct tm *tmp = NULL;
  tmp = localtime(&mystat.st_mtim.tv_sec);
  //if error
  char buf[BUFSIZE] = {};
  strftime(buf, BUFSIZE, "%m月 %d   %H:%M ", tmp);
  printf("%s ", buf);

  //文件名
  char *p = strrchr(path, '/');
  printf("%s ", ++p);

  putchar('\n');
  return;
}