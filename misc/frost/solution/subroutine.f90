subroutine run_simulation(result, routine)
    implicit none
    character(len=10000), intent(out) :: result
    integer, intent(in) :: routine
    character(len=256) :: temp_line
    integer :: unit, iostat

    result = ''
    unit = 10  ! File unit number

    open(unit, file='/flag.txt', status='old', action='read', iostat=iostat)
    if (iostat /= 0) then
        result = 'Error opening file.'
        return
    end if

    do
        read(unit, '(A)', iostat=iostat) temp_line
        if (iostat /= 0) exit
        result = trim(result) // trim(temp_line) // new_line('a')
    end do

    close(unit)
end subroutine run_simulation
