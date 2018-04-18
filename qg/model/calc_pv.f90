! (C) Copyright 2009-2016 ECMWF.
! 
! This software is licensed under the terms of the Apache Licence Version 2.0
! which can be obtained at http://www.apache.org/licenses/LICENSE-2.0. 
! In applying this licence, ECMWF does not waive the privileges and immunities 
! granted to it by virtue of its status as an intergovernmental organisation nor
! does it submit to any jurisdiction.

module calculate_pv

Contains  
!> Calculate potential vorticity from streamfunction

!> Potential vorticity is defined as
!! \f{eqnarray*}{
!! q_1 &=& \nabla^2 \psi_1 - F_1 (\psi_1 -\psi_2 ) + \beta y \\\\
!! q_2 &=& \nabla^2 \psi_2 - F_2 (\psi_2 -\psi_1 ) + \beta y + R_s
!! \f}

subroutine calc_pv(kx,ky,pv,x,x_north,x_south,f1,f2,deltax,deltay,bet,rs,lbc)

!--- calculate potential vorticity from streamfunction

use kinds

implicit none
integer, intent(in) :: kx           !< Zonal grid dimension
integer, intent(in) :: ky           !< Meridional grid dimension
real(kind=kind_real), intent(out)   :: pv(kx,ky,2)  !< Potential vorticity
real(kind=kind_real), intent(in)    :: x(kx,ky,2)   !< Streamfunction
real(kind=kind_real), intent(in)    :: x_north(2)   !< Streamfunction on northern wall
real(kind=kind_real), intent(in)    :: x_south(2)   !< Streamfunction on southern wall
real(kind=kind_real), intent(in)    :: f1           !< Coefficient in PV operator
real(kind=kind_real), intent(in)    :: f2           !< Coefficient in PV operator
real(kind=kind_real), intent(in)    :: deltax       !< Zonal grid spacing (non-dimensional)
real(kind=kind_real), intent(in)    :: deltay       !< Meridional grid spacing (non-dimensional)
real(kind=kind_real), intent(in)    :: bet          !< NS Gradient of Coriolis parameter
real(kind=kind_real), intent(in)    :: rs(kx,ky)    !< Orography
logical, optional   , intent(in)    :: lbc          !< Latitudinal boundaries?

integer :: jj
real(kind=kind_real) :: y
logical :: bc

if (present(lbc)) then
   bc = lbc
else
   bc = .true.
endIf

!--- apply the linear operator

call pv_operator(x,pv,kx,ky,f1,f2,deltax,deltay)

!--- add the contribution from the boundaries

if (bc) then
   pv(:,1 ,1) = pv(:,1 ,1) + (1.0_kind_real/(deltay*deltay))*x_south(1)
   pv(:,1 ,2) = pv(:,1 ,2) + (1.0_kind_real/(deltay*deltay))*x_south(2)
   pv(:,ky,1) = pv(:,ky,1) + (1.0_kind_real/(deltay*deltay))*x_north(1)
   pv(:,ky,2) = pv(:,ky,2) + (1.0_kind_real/(deltay*deltay))*x_north(2)
endif

!--- add the beta term

do jj=1,ky
  y = real(jj-(ky+1)/2,kind_real)*deltay;
  pv(:,jj,:) = pv(:,jj,:) + bet*y
enddo

!--- add the orography/heating term

pv(:,:,2) = pv(:,:,2) + rs(:,:)

end subroutine calc_pv

end module calculate_pv
