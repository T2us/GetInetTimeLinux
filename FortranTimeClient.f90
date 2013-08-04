!  (c) Nikolay Prodanov, January 2013, Juelich, Germany.
!  FortranTimeClient.f90
!  get internet time on Linux using C calls in the file TimedllLinux.c

      program prog
      
      implicit none   
      
      ! Variables     
      integer :: year
      integer :: month
      integer :: day
      integer :: hour
      integer :: minute 
      integer :: second

      year = -1
      month = -1
      day = -1
      hour = -1
      minute = -1
      second = -1
      
      ! Body
      call getyear(year)
      
      ! Check the output, function getyear() returns:
      ! returns -1 if socket problem
      if(year .eq. -1) then
        write(*,*) "Socket error"    
      end if    
      ! returns -2 if cannot connect to the server
      if(year .eq. -2) then
        write(*,*) "Cannot connect to server"
      end if
      ! returns -3 if connection is closed
      if(year .eq. -3) then
        write(*,*) "Connection to server is closed"
      end if
      ! returns -4 if recv failed
      if(year .eq. -4) then
        write(*,*) "Receiving data failed"
      end if
      ! returns -5 if parsing error
      if(year .eq. -5) then
        write(*,*) "Parsing error"
      end if
      
      ! get other data
      call getmonth(month)
      call getday(day)
      call gethour(hour)
      call getminute(minute)
      call getsecond(second)
      
      write(*,*) "year: ", year
      write(*,*)  "month: ", month
      write(*,*)  "day: ", day
      write(*,*)  "hour: ", hour
      write(*,*)  "minute: ", minute
      write(*,*)  "second:", second

      end program prog
