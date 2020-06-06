#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#include <stdio.h>

int main()
{
  pid_t childPID = fork();

  if ( childPID == -1 )
  {
    printf( "failed to fork child\n" );
    _exit( 1 );
  }
  else if ( childPID == 0 )
  {
    char *args[] = { "ping", "localhost", 0 };

    execv( "/bin/ping", args );
  }

  while ( 1 )
  {
    printf( "Enter 'q' to kill child process...\n" );
    char c = getchar();

    if ( c == 'q' )
    {
      kill( childPID, SIGKILL );
    }

    printf("waiting...\n");

    sleep(1);
  }

  return 0;
}  