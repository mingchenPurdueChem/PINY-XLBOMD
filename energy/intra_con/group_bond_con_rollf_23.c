/*==========================================================================*/
/*cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc*/
/*==========================================================================*/
/*                                                                          */
/*                         PI_MD:                                           */
/*             The future of simulation technology                          */
/*             ------------------------------------                         */
/*                   Module: grp_bond_con.c                                 */
/*                                                                          */
/* This routine controls the atom based group constraint routines           */
/*                                                                          */
/*==========================================================================*/
/*cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc*/
/*==========================================================================*/

#include "standard_include.h"
#include "../typ_defs/typedefs_class.h"
#include "../typ_defs/typedefs_gen.h"
#include "../typ_defs/typedefs_bnd.h"
#include "../proto_defs/proto_intra_con_local.h"
#include "../proto_defs/proto_math.h"
#include "../proto_defs/proto_friend_lib_entry.h"
#include "../proto_defs/proto_communicate_wrappers.h"
#include "../proto_defs/proto_energy_ctrl_entry.h"

#define NCON_23 2
#define NAT_23 3

/*==========================================================================*/
/*cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc*/
/*==========================================================================*/

void shake_23_rollf(GRP_BOND_CON *grp_bond_con,
                    CLATOMS_INFO *clatoms_info,CLATOMS_POS *clatoms_pos,
                    PTENS *ptens,double dt,double *aiter,
                    PAR_RAHMAN *par_rahman,int ifirst,CELL *cell,
                    CLASS_COMM_FORC_PKG *class_comm_forc_pkg)

/*==========================================================================*/
/*        Begin Routine                                                     */
    {/*Begin Routine*/
/*=======================================================================*/
/*         Local Variable declarations                                   */
  
#include "../typ_defs/typ_mask.h"

 double xl0[NCON_23+1];
 double amat[NCON_23+1][NCON_23+1],ainv[NCON_23+1][NCON_23+1];

 double r12s,r13s;
 double dxn12,dyn12,dzn12,dxn13,dyn13,dzn13;
 double dlmax,dlmax1,dlmax2,rdet_a;


 double dts;
 double dlam1,dlam2;
 double ftemp;
 double pnorm;

 int i,iii;
 int iter,igrp,*ind1,*ind2,*ind3,jtyp;
 int ktemp,ktemp1,ktemp2,ktemp3; 

/* TOP */
 double *xlam1,*xlam2;
 double *avec1,*avec2;
 double *rm1,*rm2,*rm3;
 double *rmm11,*rmm12,*rmm21,*rmm22;
 double *dxl1,*dxl2;
 double *dx12,*dy12,*dz12,*dx13,*dy13,*dz13;
 double *dxo12,*dyo12,*dzo12,*dxo13,*dyo13,*dzo13;
 double *dxo120,*dyo120,*dzo120,*dxo130,*dyo130,*dzo130;
 double **x,**y,**z,**xo,**yo,**zo;
 double *dij1,*dij2;
 double *p11,*p12,*p13,*p23,*p33,*p22;

/* Local pointers */

  double *clatoms_mass         = clatoms_info->mass;
  double *clatoms_xold         = clatoms_info->xold;
  double *clatoms_yold         = clatoms_info->yold;
  double *clatoms_zold         = clatoms_info->zold;
  double *clatoms_x            = clatoms_pos->x;
  double *clatoms_y            = clatoms_pos->y;
  double *clatoms_z            = clatoms_pos->z;
  double *clatoms_vx           = clatoms_pos->vx;
  double *clatoms_vy           = clatoms_pos->vy;
  double *clatoms_vz           = clatoms_pos->vz;

  double tol                   = grp_bond_con->tol;
  int max_iter                 = grp_bond_con->max_iter;
  int ngrp                     = grp_bond_con->num_23;
  int *grp_bond_con_j1_23      = grp_bond_con->j1_23;
  int *grp_bond_con_j2_23      = grp_bond_con->j2_23;
  int *grp_bond_con_j3_23      = grp_bond_con->j3_23;
  int *grp_bond_con_jtyp_23    = grp_bond_con->jtyp_23;
  double **grp_bond_con_eq_23  = grp_bond_con->eq_23;
  double **grp_bond_con_al_23  = grp_bond_con->al_23;

  double *pvten_inc            = ptens->pvten_inc;
  double *pvten_tmp            = ptens->pvten_tmp;
  double *pvten_tmp2           = ptens->pvten_tmp_res;

  int iperd                    = cell->iperd;
  int hmat_cons_typ            = cell->hmat_cons_typ;
  int hmat_int_typ             = cell->hmat_int_typ;

  double *roll_mtv             = par_rahman->roll_mtv;
  double *fgmat_p              = par_rahman->fgmat_p;
  double *vgmat                = par_rahman->vgmat;
  double roll_scg              = par_rahman->roll_scg;
  double mass_hm               = par_rahman->mass_hm;

  int np_forc                  = class_comm_forc_pkg->num_proc;
  int myid_forc                = class_comm_forc_pkg->myid;
  MPI_Comm comm_forc           = class_comm_forc_pkg->comm;

/*=======================================================================*/

  if(ngrp > 0){
   rm1= dvector(1,ngrp);
   rm2= dvector(1,ngrp);
   rm3= dvector(1,ngrp);
    xlam1= dvector(1,ngrp); xlam2= dvector(1,ngrp);
     dxl1= dvector(1,ngrp); dxl2= dvector(1,ngrp);
    avec1= dvector(1,ngrp); avec2= dvector(1,ngrp);
     dxo12= dvector(1,ngrp); dyo12= dvector(1,ngrp); dzo12= dvector(1,ngrp);
     dxo13= dvector(1,ngrp); dyo13= dvector(1,ngrp); dzo13= dvector(1,ngrp);
    dx12= dvector(1,ngrp); dy12= dvector(1,ngrp); dz12= dvector(1,ngrp);
    dx13= dvector(1,ngrp); dy13= dvector(1,ngrp); dz13= dvector(1,ngrp);
       x= dmatrix(1,3,1,ngrp);
       y= dmatrix(1,3,1,ngrp);
       z= dmatrix(1,3,1,ngrp);
       xo= dmatrix(1,3,1,ngrp);
       yo= dmatrix(1,3,1,ngrp);
       zo= dmatrix(1,3,1,ngrp);
      dij1= dvector(1,ngrp);
      dij2= dvector(1,ngrp);
     ind1= (int *)calloc((ngrp+1),sizeof(int));
     ind2= (int *)calloc((ngrp+1),sizeof(int));
     ind3= (int *)calloc((ngrp+1),sizeof(int));
     rmm11= dvector(1,ngrp);
     rmm12= dvector(1,ngrp);
     rmm21= dvector(1,ngrp);
     rmm22= dvector(1,ngrp);
    dxo120= dvector(1,ngrp); dyo120= dvector(1,ngrp); dzo120= dvector(1,ngrp);
    dxo130= dvector(1,ngrp); dyo130= dvector(1,ngrp); dzo130= dvector(1,ngrp);
     p11= dvector(1,ngrp);
     p12= dvector(1,ngrp);
     p13= dvector(1,ngrp);
     p22= dvector(1,ngrp);
     p33= dvector(1,ngrp);
     p23= dvector(1,ngrp);
  }/*endif*/    

/*=======================================================================*/

 dts = dt*dt;
 pnorm = 2.0/dts;
 *aiter = 0.0;
 for(i=1; i <= 9; i++){pvten_tmp[i] = 0.0;}

 if(ifirst == 2){
   for(igrp=1;igrp <= ngrp; igrp++) {
     grp_bond_con_al_23[1][igrp] = 0.0;
     grp_bond_con_al_23[2][igrp] = 0.0;
   }
 }

 for(igrp=1;igrp <= ngrp; igrp++) {
    ind1[igrp] = grp_bond_con_j1_23[igrp];
    ind2[igrp] = grp_bond_con_j2_23[igrp];
    ind3[igrp] = grp_bond_con_j3_23[igrp];
 }/*end for*/

 for(igrp=1;igrp <= ngrp; igrp++) {
    ktemp= ind1[igrp];
    x[1][igrp] = clatoms_x[ktemp];
    y[1][igrp] = clatoms_y[ktemp];
    z[1][igrp] = clatoms_z[ktemp];
    rm1[igrp] = 1.0/clatoms_mass[ktemp];
  }/*end for*/

 for(igrp=1;igrp <= ngrp; igrp++) {
    ktemp= ind2[igrp];
    x[2][igrp] = clatoms_x[ktemp];
    y[2][igrp] = clatoms_y[ktemp];
    z[2][igrp] = clatoms_z[ktemp];
    rm2[igrp] = 1.0/clatoms_mass[ktemp];
  }/*end for*/

 for(igrp=1;igrp <= ngrp; igrp++) {
    ktemp= ind3[igrp];
    x[3][igrp] = clatoms_x[ktemp];
    y[3][igrp] = clatoms_y[ktemp];
    z[3][igrp] = clatoms_z[ktemp];
    rm3[igrp] = 1.0/clatoms_mass[ktemp];
  }/*end for*/

 for(igrp=1;igrp <= ngrp; igrp++) {
    ktemp= ind1[igrp];
    xo[1][igrp] = clatoms_xold[ktemp];
    yo[1][igrp] = clatoms_yold[ktemp];
    zo[1][igrp] = clatoms_zold[ktemp];
  }/*end for*/

 for(igrp=1;igrp <= ngrp; igrp++) {
    ktemp= ind2[igrp];
    xo[2][igrp] = clatoms_xold[ktemp];
    yo[2][igrp] = clatoms_yold[ktemp];
    zo[2][igrp] = clatoms_zold[ktemp];
  }/*end for*/

 for(igrp=1;igrp <= ngrp; igrp++) {
    ktemp= ind3[igrp];
    xo[3][igrp] = clatoms_xold[ktemp];
    yo[3][igrp] = clatoms_yold[ktemp];
    zo[3][igrp] = clatoms_zold[ktemp];
  }/*end for*/

 for(igrp=1;igrp <= ngrp; igrp++) {
    jtyp = grp_bond_con_jtyp_23[igrp];
    dij1[igrp] = grp_bond_con_eq_23[1][jtyp];
    dij2[igrp] = grp_bond_con_eq_23[2][jtyp];
 }/*end for*/

/* Initial Guess for multipliers */
 for(igrp=1;igrp <= ngrp; igrp++) {
    rmm11[igrp] = -(rm1[igrp]+rm2[igrp]);
    rmm12[igrp] = -rm1[igrp];
    rmm21[igrp] = -rm1[igrp];
    rmm22[igrp] = -(rm1[igrp]+rm3[igrp]);
 }

 for(igrp=1;igrp <= ngrp; igrp++) {
    dx12[igrp] = x[1][igrp]-x[2][igrp];
    dy12[igrp] = y[1][igrp]-y[2][igrp];
    dz12[igrp] = z[1][igrp]-z[2][igrp];
  }

 for(igrp=1;igrp <= ngrp; igrp++) {
    dx13[igrp] = x[1][igrp]-x[3][igrp];
    dy13[igrp] = y[1][igrp]-y[3][igrp];
    dz13[igrp] = z[1][igrp]-z[3][igrp];
  }

 for(igrp=1;igrp <= ngrp; igrp++) {
    dxo120[igrp] = xo[1][igrp]-xo[2][igrp];
    dyo120[igrp] = yo[1][igrp]-yo[2][igrp];
    dzo120[igrp] = zo[1][igrp]-zo[2][igrp];
  }

 for(igrp=1;igrp <= ngrp; igrp++) {
    dxo130[igrp] = xo[1][igrp]-xo[3][igrp];
    dyo130[igrp] = yo[1][igrp]-yo[3][igrp];
    dzo130[igrp] = zo[1][igrp]-zo[3][igrp];
  }


/*  dxo120[igrp] = (xo[1]-xo[2]);
  dyo120[igrp] = (yo[1]-yo[2]);
  dzo120[igrp] = (zo[1]-zo[2]);
  dxo130[igrp] = (xo[1]-xo[3]);
  dyo130[igrp] = (yo[1]-yo[3]);
  dzo130[igrp] = (zo[1]-zo[3]); */

 for(igrp=1;igrp <= ngrp; igrp++) {
  dxo12[igrp] = (dxo120[igrp])*roll_mtv[1] 
              + (dyo120[igrp])*roll_mtv[2]
              + (dzo120[igrp])*roll_mtv[3];
  dyo12[igrp] = (dxo120[igrp])*roll_mtv[4] 
              + (dyo120[igrp])*roll_mtv[5]
              + (dzo120[igrp])*roll_mtv[6];
  dzo12[igrp] = (dxo120[igrp])*roll_mtv[7] 
              + (dyo120[igrp])*roll_mtv[8]
              + (dzo120[igrp])*roll_mtv[9];
 }/*end for*/

 for(igrp=1;igrp <= ngrp; igrp++) {
  dxo13[igrp] = (dxo130[igrp])*roll_mtv[1] 
              + (dyo130[igrp])*roll_mtv[2]
              + (dzo130[igrp])*roll_mtv[3];
  dyo13[igrp] = (dxo130[igrp])*roll_mtv[4] 
              + (dyo130[igrp])*roll_mtv[5]
              + (dzo130[igrp])*roll_mtv[6];
  dzo13[igrp] = (dxo130[igrp])*roll_mtv[7] 
              + (dyo130[igrp])*roll_mtv[8]
              + (dzo130[igrp])*roll_mtv[9];
 }/*end for*/

  
 for(igrp=1;igrp <= ngrp; igrp++) {
   r12s = dx12[igrp]*dx12[igrp] + dy12[igrp]*dy12[igrp]
        + dz12[igrp]*dz12[igrp];
   r13s = dx13[igrp]*dx13[igrp] + dy13[igrp]*dy13[igrp]
        + dz13[igrp]*dz13[igrp];
   avec1[igrp] = dij1[igrp]*dij1[igrp] - r12s;
   avec2[igrp] = dij2[igrp]*dij2[igrp] - r13s;
  }/*end for*/

/* Initial Guess for multipliers */
  if(ifirst==2||ifirst==0){
 for(igrp=1;igrp <= ngrp; igrp++) {

   amat[1][1] = 2.0*rmm11[igrp]*(dx12[igrp]*dxo12[igrp]
              + dy12[igrp]*dyo12[igrp]
              + dz12[igrp]*dzo12[igrp]);
   amat[1][2] = 2.0*rmm12[igrp]*(dx12[igrp]*dxo13[igrp]
              + dy12[igrp]*dyo13[igrp]
              + dz12[igrp]*dzo13[igrp]);
   amat[2][1] = 2.0*rmm21[igrp]*(dx13[igrp]*dxo12[igrp]
              + dy13[igrp]*dyo12[igrp]
              + dz13[igrp]*dzo12[igrp]);
   amat[2][2] = 2.0*rmm22[igrp]*(dx13[igrp]*dxo13[igrp]
              + dy13[igrp]*dyo13[igrp]
              + dz13[igrp]*dzo13[igrp]);

   rdet_a = 1.0/(amat[1][1]*amat[2][2] - amat[1][2]*amat[2][1]);
   ainv[1][1] =  amat[2][2]*rdet_a;
   ainv[1][2] = -amat[1][2]*rdet_a;
   ainv[2][1] = -amat[2][1]*rdet_a;
   ainv[2][2] =  amat[1][1]*rdet_a;

   xlam1[igrp] = ainv[1][1]*avec1[igrp] + ainv[1][2]*avec2[igrp];
   xlam2[igrp] = ainv[2][1]*avec1[igrp] + ainv[2][2]*avec2[igrp];
  } /*end for*/
  }else{
  for(igrp=1;igrp <= ngrp; igrp++) {
   xlam1[igrp] = grp_bond_con_al_23[1][igrp];
   xlam2[igrp] = grp_bond_con_al_23[2][igrp];
   grp_bond_con_al_23[1][igrp] = 0.0;
   grp_bond_con_al_23[2][igrp] = 0.0;
   } /*end for*/
  }/*endif*/
 
/* Iterative do loop for multiplier */

 if(ngrp > 0){

  iter = 0;
  do {
   ++iter;
   if(iter > max_iter) {
    printf("$$$$$$$$$$$$$$$$$$$$_WARNING_$$$$$$$$$$$$$$$$$$$$\n");
    printf("Group Shake 23 not converged after %d iterations.\n",max_iter);
    printf("The present tolerance is %g \n",dlmax);
    printf("The desired tolerance is %g \n",tol);
    printf("$$$$$$$$$$$$$$$$$$$$_WARNING_$$$$$$$$$$$$$$$$$$$$\n");
    fflush(stdout);
    break;
   }/*endif*/
/* Get elements of matrix */
 for(igrp=1;igrp <= ngrp; igrp++) {

  dxn12 = 2.0*dx12[igrp]
        + rmm11[igrp]*xlam1[igrp]*dxo12[igrp]
        + rmm12[igrp]*xlam2[igrp]*dxo13[igrp];
  dyn12 = 2.0*dy12[igrp]
        + rmm11[igrp]*xlam1[igrp]*dyo12[igrp]
        + rmm12[igrp]*xlam2[igrp]*dyo13[igrp];
  dzn12 = 2.0*dz12[igrp]
        + rmm11[igrp]*xlam1[igrp]*dzo12[igrp]
        + rmm12[igrp]*xlam2[igrp]*dzo13[igrp];
  dxn13 = 2.0*dx13[igrp]
        + rmm21[igrp]*xlam1[igrp]*dxo12[igrp]
        + rmm22[igrp]*xlam2[igrp]*dxo13[igrp];
  dyn13 = 2.0*dy13[igrp]
        + rmm21[igrp]*xlam1[igrp]*dyo12[igrp]
        + rmm22[igrp]*xlam2[igrp]*dyo13[igrp];
  dzn13 = 2.0*dz13[igrp]
        + rmm21[igrp]*xlam1[igrp]*dzo12[igrp]
        + rmm22[igrp]*xlam2[igrp]*dzo13[igrp];

   amat[1][1] = rmm11[igrp]*(dxn12*dxo12[igrp]
              + dyn12*dyo12[igrp]
              + dzn12*dzo12[igrp]);
   amat[1][2] = rmm12[igrp]*(dxn12*dxo13[igrp]
              + dyn12*dyo13[igrp]
              + dzn12*dzo13[igrp]);
   amat[2][1] = rmm21[igrp]*(dxn13*dxo12[igrp]
              + dyn13*dyo12[igrp]
              + dzn13*dzo12[igrp]);
   amat[2][2] = rmm22[igrp]*(dxn13*dxo13[igrp]
              + dyn13*dyo13[igrp]
              + dzn13*dzo13[igrp]);

   rdet_a = 1.0/(amat[1][1]*amat[2][2] - amat[1][2]*amat[2][1]);
   ainv[1][1] =  amat[2][2]*rdet_a;
   ainv[1][2] = -amat[1][2]*rdet_a;
   ainv[2][1] = -amat[2][1]*rdet_a;
   ainv[2][2] =  amat[1][1]*rdet_a;

   xl0[1] = xlam1[igrp];
   xl0[2] = xlam2[igrp];

   xlam1[igrp] = ainv[1][1]*avec1[igrp] + ainv[1][2]*avec2[igrp];
   xlam2[igrp] = ainv[2][1]*avec1[igrp] + ainv[2][2]*avec2[igrp];

   dxl1[igrp] = fabs(xlam1[igrp]-xl0[1]);
   dxl2[igrp] = fabs(xlam2[igrp]-xl0[2]);
 } /* end loop over groups */
/* test for convergence */
      dlmax1= dxl1[1];
      dlmax2= dxl2[1];

  for(igrp=2;igrp <= ngrp; igrp++) {
   dlmax1= (dlmax1> dxl1[igrp] ? dlmax1 : dxl1[igrp]);
   dlmax2= (dlmax2> dxl2[igrp] ? dlmax2 : dxl2[igrp]);
  }/* end loop over groups */
   dlmax = (dlmax1 > dlmax2 ? dlmax1:dlmax2);
  } while(dlmax > tol);
  *aiter += (double) iter;

 }/*endif ngrp > 0*/

/* position update */
#ifndef NO_PRAGMA
#pragma IVDEP
#endif
  for(igrp=1;igrp <= ngrp; igrp++) {
    ktemp= ind1[igrp];
    clatoms_x[ktemp] -= (xlam1[igrp]*dxo12[igrp] 
                      + xlam2[igrp]*dxo13[igrp])*rm1[igrp];
    clatoms_y[ktemp] -= (xlam1[igrp]*dyo12[igrp]
                      + xlam2[igrp]*dyo13[igrp])*rm1[igrp];
    clatoms_z[ktemp] -= (xlam1[igrp]*dzo12[igrp] 
                      + xlam2[igrp]*dzo13[igrp])*rm1[igrp];
 }/*end for*/


#ifndef NO_PRAGMA
#pragma IVDEP
#endif
  for(igrp=1;igrp <= ngrp; igrp++) {
    ktemp= ind2[igrp];
  clatoms_x[ktemp] += xlam1[igrp]*dxo12[igrp]*rm2[igrp];
  clatoms_y[ktemp] += xlam1[igrp]*dyo12[igrp]*rm2[igrp];
  clatoms_z[ktemp] += xlam1[igrp]*dzo12[igrp]*rm2[igrp];
 }/*end for*/
 

#ifndef NO_PRAGMA
#pragma IVDEP
#endif
  for(igrp=1;igrp <= ngrp; igrp++) {
    ktemp= ind3[igrp];
  clatoms_x[ktemp] += xlam2[igrp]*dxo13[igrp]*rm3[igrp];
  clatoms_y[ktemp] += xlam2[igrp]*dyo13[igrp]*rm3[igrp];
  clatoms_z[ktemp] += xlam2[igrp]*dzo13[igrp]*rm3[igrp];
}/*end for*/

/* Velocity update */

#ifndef NO_PRAGMA
#pragma IVDEP
#endif
  for(igrp=1;igrp <= ngrp; igrp++) {
    ktemp= ind1[igrp];
  clatoms_vx[ktemp]-=(xlam1[igrp]*dxo12[igrp]+xlam2[igrp]*dxo13[igrp])*rm1[igrp]/dt;
  clatoms_vy[ktemp]-=(xlam1[igrp]*dyo12[igrp]+xlam2[igrp]*dyo13[igrp])*rm1[igrp]/dt;
  clatoms_vz[ktemp]-=(xlam1[igrp]*dzo12[igrp]+xlam2[igrp]*dzo13[igrp])*rm1[igrp]/dt;
 }/*end for*/


#ifndef NO_PRAGMA
#pragma IVDEP
#endif
  for(igrp=1;igrp <= ngrp; igrp++) {
    ktemp= ind2[igrp];
  clatoms_vx[ktemp] += xlam1[igrp]*dxo120[igrp]*rm2[igrp]/dt;
  clatoms_vy[ktemp] += xlam1[igrp]*dyo120[igrp]*rm2[igrp]/dt;
  clatoms_vz[ktemp] += xlam1[igrp]*dzo120[igrp]*rm2[igrp]/dt;
}/*end for*/



#ifndef NO_PRAGMA
#pragma IVDEP
#endif
  for(igrp=1;igrp <= ngrp; igrp++) {
    ktemp= ind3[igrp];
  clatoms_vx[ktemp] += xlam2[igrp]*dxo130[igrp]*rm3[igrp]/dt;
  clatoms_vy[ktemp] += xlam2[igrp]*dyo130[igrp]*rm3[igrp]/dt;
  clatoms_vz[ktemp] += xlam2[igrp]*dzo130[igrp]*rm3[igrp]/dt;
 }/*end for*/
 



/* Pressure tensor update */
/* Compute difference vectors: unscaled old distances !!!*/

  for(igrp=1;igrp <= ngrp; igrp++) {
    p11[igrp] = xlam1[igrp]*dxo120[igrp]*dxo120[igrp] 
              + xlam2[igrp]*dxo130[igrp]*dxo130[igrp];
    p22[igrp] = xlam1[igrp]*dyo120[igrp]*dyo120[igrp] 
              + xlam2[igrp]*dyo130[igrp]*dyo130[igrp];
    p33[igrp] = xlam1[igrp]*dzo120[igrp]*dzo120[igrp] 
              + xlam2[igrp]*dzo130[igrp]*dzo130[igrp];
    p12[igrp] = xlam1[igrp]*dxo120[igrp]*dyo120[igrp] 
              + xlam2[igrp]*dxo130[igrp]*dyo130[igrp];
    p13[igrp] = xlam1[igrp]*dxo120[igrp]*dzo120[igrp] 
              + xlam2[igrp]*dxo130[igrp]*dzo130[igrp];
    p23[igrp] = xlam1[igrp]*dyo120[igrp]*dzo120[igrp] 
              + xlam2[igrp]*dyo130[igrp]*dzo130[igrp];
}/*end for*/

#ifndef NO_PRAGMA
#pragma IVDEP
#endif
  for(igrp=1;igrp <= ngrp; igrp++) {
    pvten_tmp[1] -= (p11[igrp]*pnorm); 
    pvten_tmp[2] -= (p12[igrp]*pnorm);
    pvten_tmp[3] -= (p13[igrp]*pnorm);
    pvten_tmp[4] -= (p12[igrp]*pnorm);
    pvten_tmp[5] -= (p22[igrp]*pnorm);
    pvten_tmp[6] -= (p23[igrp]*pnorm);
    pvten_tmp[7] -= (p13[igrp]*pnorm);
    pvten_tmp[8] -= (p23[igrp]*pnorm);
    pvten_tmp[9] -= (p33[igrp]*pnorm);
  }/*end for*/

/* Save multiplier */

  for(igrp=1;igrp <= ngrp; igrp++) {
     grp_bond_con_al_23[1][igrp] += xlam1[igrp];
 } /* end for */

/*=======================================================================*/
/*  IV)Allreduce pvten_tmp     */

  if(np_forc > 1 ){
   for(i=1;i<=9;i++){
    pvten_tmp2[i] = pvten_tmp[i];
   }/*endfor*/
   Allreduce(&(pvten_tmp2[1]), &(pvten_tmp[1]),9,MPI_DOUBLE,
                   MPI_SUM,0,comm_forc);
  }/*endif*/

  pvten_inc[1] += pvten_tmp[1];
  pvten_inc[2] += pvten_tmp[2];
  pvten_inc[3] += pvten_tmp[3];
  pvten_inc[4] += pvten_tmp[4];
  pvten_inc[5] += pvten_tmp[5];
  pvten_inc[6] += pvten_tmp[6];
  pvten_inc[7] += pvten_tmp[7];
  pvten_inc[8] += pvten_tmp[8];
  pvten_inc[9] += pvten_tmp[9];

 if(ifirst == 0){
   constr_cell_mat(iperd,hmat_cons_typ,hmat_int_typ,pvten_tmp);
   for(i=1;i<=9;i++){
     fgmat_p[i]+=pvten_tmp[i];    
     vgmat[i]  +=pvten_tmp[i]*(roll_scg)
                            *0.5*dt/(mass_hm);
   }/*endfor*/
 }/*endif*/
  
/* free locally assigned memory */
 if(ngrp > 0){
    free_dvector(rm1,1,ngrp);
    free_dvector(rm2,1,ngrp);
    free_dvector(rm3,1,ngrp);

    free_dvector(xlam1,1,ngrp); free_dvector(xlam2,1,ngrp);

    free_dvector(dxl1,1,ngrp); free_dvector(dxl2,1,ngrp);

    free_dvector(avec1,1,ngrp); free_dvector(avec2,1,ngrp);

    free_dvector(dxo12,1,ngrp);
    free_dvector(dyo12,1,ngrp);
    free_dvector(dzo12,1,ngrp);

    free_dvector(dxo13,1,ngrp);
    free_dvector(dyo13,1,ngrp);
    free_dvector(dzo13,1,ngrp);

    free_dvector(dx12,1,ngrp);
    free_dvector(dy12,1,ngrp);
    free_dvector(dz12,1,ngrp);

    free_dvector(dx13,1,ngrp);
    free_dvector(dy13,1,ngrp);
    free_dvector(dz13,1,ngrp);

    free_dmatrix(x,1,3,1,ngrp);
    free_dmatrix(y,1,3,1,ngrp);
    free_dmatrix(z,1,3,1,ngrp);

    free_dmatrix(xo,1,3,1,ngrp);
    free_dmatrix(yo,1,3,1,ngrp);
    free_dmatrix(zo,1,3,1,ngrp);

    free_dvector(dij1,1,ngrp);
    free_dvector(dij2,1,ngrp);

    free(ind1);
    free(ind2);
    free(ind3);

    free_dvector(rmm11,1,ngrp);
    free_dvector(rmm12,1,ngrp);
    free_dvector(rmm21,1,ngrp);
    free_dvector(rmm22,1,ngrp);

    free_dvector(dxo120,1,ngrp);
    free_dvector(dyo120,1,ngrp);
    free_dvector(dzo120,1,ngrp);

    free_dvector(dxo130,1,ngrp);
    free_dvector(dyo130,1,ngrp);
    free_dvector(dzo130,1,ngrp);

    free_dvector(p11,1,ngrp);
    free_dvector(p12,1,ngrp);
    free_dvector(p13,1,ngrp);
    free_dvector(p22,1,ngrp);
    free_dvector(p33,1,ngrp);
    free_dvector(p23,1,ngrp);
 }/*endif*/

/*=======================================================================*/
   }/* end routine */
/*=======================================================================*/





/*==========================================================================*/
/*cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc*/
/*==========================================================================*/

void rattle_23_rollf(GRP_BOND_CON *grp_bond_con,
                     CLATOMS_INFO *clatoms_info,CLATOMS_POS *clatoms_pos,
                     PTENS *ptens,double dt,
                     PAR_RAHMAN *par_rahman,int ifirst,CELL *cell,
                     CLASS_COMM_FORC_PKG *class_comm_forc_pkg)

/*==========================================================================*/
/*        Begin Routine                                                     */
   {/* Begin routine */
/*==========================================================================*/
/*         Local Variable declarations                                      */

#include "../typ_defs/typ_mask.h"

  int i,igrp,*ind1,*ind2,*ind3,jtyp,n;
  int  ktemp,ktemp1,ktemp2,ktemp3;

  double avec[NCON_23+1];
  double amat[NCON_23+1][NCON_23+1],ainv[NCON_23+1][NCON_23+1];
  double rmu1,rmu2;
 
  double dvx12,dvy12,dvz12,dvx13,dvy13,dvz13;
  double r12s,r13s,dot23,dot2t2,dot3t3;
  double rdet_a;

  double roll_sci,dlam1,dlam2;
  double f_lnv_inc;

  double *p11,*p12,*p13,*p22,*p23,*p33;
  double **x,**y,**z,**vx,**vy,**vz;
  double *rm1,*rm2,*rm3;
  double *dx12,*dy12,*dz12,*dx13,*dy13,*dz13;
  double **xlam;

  double pnorm;
  double roll_mtvvi[10],rolli_by_vg[10];
  double det_roll_mtvv;

/*=======================================================================*/
   
/* Local pointers */

  double *clatoms_mass         = clatoms_info->mass;
  double *clatoms_x            = clatoms_pos->x;
  double *clatoms_y            = clatoms_pos->y;
  double *clatoms_z            = clatoms_pos->z;
  double *clatoms_vx           = clatoms_pos->vx;
  double *clatoms_vy           = clatoms_pos->vy;
  double *clatoms_vz           = clatoms_pos->vz;
  double *clatoms_roll_sc      = clatoms_info->roll_sc;

  int ngrp                     = grp_bond_con->num_23;
  int *grp_bond_con_j1_23      = grp_bond_con->j1_23;
  int *grp_bond_con_j2_23      = grp_bond_con->j2_23;
  int *grp_bond_con_j3_23      = grp_bond_con->j3_23;
  int *grp_bond_con_jtyp_23    = grp_bond_con->jtyp_23;
  double **grp_bond_con_al_23  = grp_bond_con->al_23;

  double *pvten_inc            = ptens->pvten_inc;
  double *pvten_tmp            = ptens->pvten_tmp;
  double *pvten_tmp2           = ptens->pvten_tmp_res;

  double *roll_mtvv            = par_rahman->roll_mtvv;
  double *vgmat_g              = par_rahman->vgmat_g;
  double *fgmat_p              = par_rahman->fgmat_p;
  double roll_scg              = par_rahman->roll_scg;
  double mass_hm               = par_rahman->mass_hm;

  int iperd                    = cell->iperd;
  int hmat_cons_typ            = cell->hmat_cons_typ;
  int hmat_int_typ             = cell->hmat_int_typ;

  int np_forc                  = class_comm_forc_pkg->num_proc;
  int myid_forc                = class_comm_forc_pkg->myid;
  MPI_Comm comm_forc           = class_comm_forc_pkg->comm;

/*=======================================================================*/
/* assign local memory */ 
  if(ngrp > 0){
   p11= dvector(1,ngrp); p12= dvector(1,ngrp);
   p13= dvector(1,ngrp); p22= dvector(1,ngrp);
   p23= dvector(1,ngrp); p33= dvector(1,ngrp);
   ind1 = (int *)calloc((ngrp+1),sizeof(int));
   ind2 = (int *)calloc((ngrp+1),sizeof(int));
   ind3 = (int *)calloc((ngrp+1),sizeof(int));
      x = dmatrix(1,3,1,ngrp); 
      y = dmatrix(1,3,1,ngrp); 
      z = dmatrix(1,3,1,ngrp); 
     vx = dmatrix(1,3,1,ngrp); 
     vy = dmatrix(1,3,1,ngrp); 
     vz = dmatrix(1,3,1,ngrp); 
  rm1 = dvector(1,ngrp);
  rm2 = dvector(1,ngrp);
  rm3 = dvector(1,ngrp);
     dx12= dvector(1,ngrp);
     dy12= dvector(1,ngrp);
     dz12= dvector(1,ngrp);
     dx13= dvector(1,ngrp);
     dy13= dvector(1,ngrp);
     dz13= dvector(1,ngrp);
    xlam= dmatrix(1,2,1,ngrp);
  }/*endif*/

/*=======================================================================*/
 pnorm = 2.0/dt;

 n=3;
 gethinv(roll_mtvv,roll_mtvvi,&det_roll_mtvv,n);
 matmul_tt(roll_mtvvi,vgmat_g,rolli_by_vg,n);

 pvten_tmp[1] = 0.0;
 pvten_tmp[2] = 0.0;
 pvten_tmp[3] = 0.0;
 pvten_tmp[4] = 0.0;
 pvten_tmp[5] = 0.0;
 pvten_tmp[6] = 0.0;
 pvten_tmp[7] = 0.0;
 pvten_tmp[8] = 0.0;
 pvten_tmp[9] = 0.0;


 for(igrp=1;igrp <= ngrp; igrp++) {
    ind1[igrp] = grp_bond_con_j1_23[igrp];
    ind2[igrp] = grp_bond_con_j2_23[igrp];
    ind3[igrp] = grp_bond_con_j3_23[igrp];
 }/*end for*/

 for(igrp=1;igrp <= ngrp; igrp++) {
     ktemp= ind1[igrp];
     x[1][igrp] = clatoms_x[ktemp];
     y[1][igrp] = clatoms_y[ktemp];
     z[1][igrp] = clatoms_z[ktemp];
     rm1[igrp] = 1.0/clatoms_mass[ktemp];
 }/*end for*/

 for(igrp=1;igrp <= ngrp; igrp++) {
     ktemp= ind2[igrp];
     x[2][igrp] = clatoms_x[ktemp];
     y[2][igrp] = clatoms_y[ktemp];
     z[2][igrp] = clatoms_z[ktemp];
     rm2[igrp] = 1.0/clatoms_mass[ktemp];
 }/*end for*/

 for(igrp=1;igrp <= ngrp; igrp++) {
     ktemp= ind3[igrp];
     x[3][igrp] = clatoms_x[ktemp];
     y[3][igrp] = clatoms_y[ktemp];
     z[3][igrp] = clatoms_z[ktemp];
     rm3[igrp] = 1.0/clatoms_mass[ktemp];
 }/*end for*/


 for(igrp=1;igrp <= ngrp; igrp++) {
    ktemp= ind1[igrp];
    ktemp3= ind3[igrp];
   roll_sci=1.0/clatoms_roll_sc[ktemp3];/*all roll scales the same in same cons*/

  vx[1][igrp] = clatoms_vx[ktemp]
        + (x[1][igrp]*rolli_by_vg[1] +y[1][igrp]*rolli_by_vg[2] 
         + z[1][igrp]*rolli_by_vg[3]) *roll_sci;
  vy[1][igrp] = clatoms_vy[ktemp]
        + (x[1][igrp]*rolli_by_vg[4] +y[1][igrp]*rolli_by_vg[5] 
         + z[1][igrp]*rolli_by_vg[6]) *roll_sci;
  vz[1][igrp] = clatoms_vz[ktemp]
        + (x[1][igrp]*rolli_by_vg[7] +y[1][igrp]*rolli_by_vg[8] 
         + z[1][igrp]*rolli_by_vg[9]) *roll_sci;
}/*end for*/


 for(igrp=1;igrp <= ngrp; igrp++) {
    ktemp= ind2[igrp];
    ktemp3= ind3[igrp];
   roll_sci=1.0/clatoms_roll_sc[ktemp3];/*all roll scales the same in same cons*/

  vx[2][igrp] = clatoms_vx[ktemp]
        + (x[2][igrp]*rolli_by_vg[1] +y[2][igrp]*rolli_by_vg[2] 
         + z[2][igrp]*rolli_by_vg[3]) *roll_sci;
  vy[2][igrp] = clatoms_vy[ktemp]
        + (x[2][igrp]*rolli_by_vg[4] +y[2][igrp]*rolli_by_vg[5] 
         + z[2][igrp]*rolli_by_vg[6]) *roll_sci;
  vz[2][igrp] = clatoms_vz[ktemp]
        + (x[2][igrp]*rolli_by_vg[7] +y[2][igrp]*rolli_by_vg[8] 
         + z[2][igrp]*rolli_by_vg[9]) *roll_sci;
}/*end for*/

 for(igrp=1;igrp <= ngrp; igrp++) {
    ktemp= ind3[igrp];
   roll_sci=1.0/clatoms_roll_sc[ktemp];/*all roll scales the same in same cons*/

  vx[3][igrp] = clatoms_vx[ktemp]
        + (x[3][igrp]*rolli_by_vg[1] +y[3][igrp]*rolli_by_vg[2] 
         + z[3][igrp]*rolli_by_vg[3]) *roll_sci;
  vy[3][igrp] = clatoms_vy[ktemp]
        + (x[3][igrp]*rolli_by_vg[4] +y[3][igrp]*rolli_by_vg[5] 
         + z[3][igrp]*rolli_by_vg[6]) *roll_sci;
  vz[3][igrp] = clatoms_vz[ktemp]
        + (x[3][igrp]*rolli_by_vg[7] +y[3][igrp]*rolli_by_vg[8] 
         + z[3][igrp]*rolli_by_vg[9]) *roll_sci;
}/*end for*/

/* Define useful constants */

 for(igrp=1;igrp <= ngrp; igrp++) {
  rmu1= rm1[igrp] + rm2[igrp];
  rmu2= rm1[igrp] + rm3[igrp];
  
  dx12[igrp] = (x[1][igrp]-x[2][igrp]);
  dy12[igrp] = (y[1][igrp]-y[2][igrp]);
  dz12[igrp] = (z[1][igrp]-z[2][igrp]);
  r12s = dx12[igrp]*dx12[igrp] + dy12[igrp]*dy12[igrp] + dz12[igrp]*dz12[igrp];

  dx13[igrp] = (x[1][igrp]-x[3][igrp]);
  dy13[igrp] = (y[1][igrp]-y[3][igrp]);
  dz13[igrp] = (z[1][igrp]-z[3][igrp]);
  r13s = dx13[igrp]*dx13[igrp] + dy13[igrp]*dy13[igrp] + dz13[igrp]*dz13[igrp];

  dvx12 = (vx[1][igrp]-vx[2][igrp]);
  dvy12 = (vy[1][igrp]-vy[2][igrp]);
  dvz12 = (vz[1][igrp]-vz[2][igrp]);

  dvx13 = (vx[1][igrp]-vx[3][igrp]);
  dvy13 = (vy[1][igrp]-vy[3][igrp]);
  dvz13 = (vz[1][igrp]-vz[3][igrp]);

/* Get elements of vector */

  dot2t2 = dx12[igrp]*dvx12 + dy12[igrp]*dvy12 + dz12[igrp]*dvz12;
  dot3t3 = dx13[igrp]*dvx13 + dy13[igrp]*dvy13 + dz13[igrp]*dvz13;
  dot23  = dx12[igrp]*dx13[igrp]  + dy12[igrp]*dy13[igrp]  + dz12[igrp]*dz13[igrp];

  avec[1] = dot2t2;
  avec[2] = dot3t3;

/* Get elements of matrix */

  amat[1][1] = rmu1*r12s;
  amat[1][2] = rm1[igrp]*dot23;
  amat[2][1] = amat[1][2];
  amat[2][2] = rmu2*r12s;

  rdet_a = 1.0/(amat[1][1]*amat[2][2] - amat[1][2]*amat[2][1]);
  ainv[1][1] =  amat[2][2]*rdet_a;
  ainv[1][2] = -amat[1][2]*rdet_a;
  ainv[2][1] = -amat[2][1]*rdet_a;
  ainv[2][2] =  amat[1][1]*rdet_a;

  xlam[1][igrp] = ainv[1][1]*avec[1] + ainv[1][2]*avec[2];
  xlam[2][igrp] = ainv[2][1]*avec[1] + ainv[2][2]*avec[2];
 }/*end for*/

#ifndef NO_PRAGMA
#pragma IVDEP
#endif
 for(igrp=1;igrp <= ngrp; igrp++) {
   double xlam1,xlam2;
   double dx_12,dx_13;
   double dy_12,dy_13;
   double dz_12,dz_13;
   int ktemp1,ktemp2,ktemp3;

    xlam1= xlam[1][igrp];
    xlam2= xlam[2][igrp];

    dx_12= dx12[igrp]; dx_13= dx13[igrp];
    dy_12= dy12[igrp]; dy_13= dy13[igrp];
    dz_12= dz12[igrp]; dz_13= dz13[igrp];

    ktemp1=ind1[igrp]; ktemp2=ind2[igrp]; ktemp3=ind3[igrp];

   clatoms_vx[ktemp1] -= (xlam1*dx_12 + xlam2*dx_13)*rm1[igrp];
   clatoms_vy[ktemp1] -= (xlam1*dy_12 + xlam2*dy_13)*rm1[igrp];
   clatoms_vz[ktemp1] -= (xlam1*dz_12 + xlam2*dz_13)*rm1[igrp];


   clatoms_vx[ktemp2] += xlam1*dx_12*rm2[igrp];
   clatoms_vy[ktemp2] += xlam1*dy_12*rm2[igrp];
   clatoms_vz[ktemp2] += xlam1*dz_12*rm2[igrp];

   clatoms_vx[ktemp3] += xlam2*dx_13*rm3[igrp];
   clatoms_vy[ktemp3] += xlam2*dy_13*rm3[igrp];
   clatoms_vz[ktemp3] += xlam2*dz_13*rm3[igrp];

/* Pressure Tensor update */
    p11[igrp] = xlam1*dx_12*dx_12 + xlam2*dx_13*dx_13;
    p22[igrp] = xlam1*dy_12*dy_12 + xlam2*dy_13*dy_13;
    p33[igrp] = xlam1*dz_12*dz_12 + xlam2*dz_13*dz_13;
    p12[igrp] = xlam1*dx_12*dy_12 + xlam2*dx_13*dy_13;
    p13[igrp] = xlam1*dx_12*dz_12 + xlam2*dx_13*dz_13;
    p23[igrp] = xlam1*dy_12*dz_12 + xlam2*dy_13*dz_13;
}/*end for*/

#ifndef NO_PRAGMA
#pragma IVDEP
#endif
 for(igrp=1;igrp <= ngrp; igrp++) {
    pvten_tmp[1] -= (p11[igrp]*pnorm);
    pvten_tmp[2] -= (p12[igrp]*pnorm);
    pvten_tmp[3] -= (p13[igrp]*pnorm);
    pvten_tmp[4] -= (p12[igrp]*pnorm);
    pvten_tmp[5] -= (p22[igrp]*pnorm);
    pvten_tmp[6] -= (p23[igrp]*pnorm);
    pvten_tmp[7] -= (p13[igrp]*pnorm);
    pvten_tmp[8] -= (p23[igrp]*pnorm);
    pvten_tmp[9] -= (p33[igrp]*pnorm);
 }/*end for*/

/* Save multiplier */
 for(igrp=1;igrp <= ngrp; igrp++) {
  grp_bond_con_al_23[1][igrp] = xlam[1][igrp];
  grp_bond_con_al_23[2][igrp] = xlam[2][igrp];
 }/* end for igrp */
   

/*=======================================================================*/
/*  IV)Allreduce pvten_tmp     */

  if(np_forc > 1 ){
   for(i=1;i<=9;i++){
    pvten_tmp2[i] = pvten_tmp[i];
   }/*endfor*/
   Allreduce(&(pvten_tmp2[1]), &(pvten_tmp[1]),9,MPI_DOUBLE,
                   MPI_SUM,0,comm_forc);
  }/*endif*/

  pvten_inc[1] += pvten_tmp[1];
  pvten_inc[2] += pvten_tmp[2];
  pvten_inc[3] += pvten_tmp[3];
  pvten_inc[4] += pvten_tmp[4];
  pvten_inc[5] += pvten_tmp[5];
  pvten_inc[6] += pvten_tmp[6];
  pvten_inc[7] += pvten_tmp[7];
  pvten_inc[8] += pvten_tmp[8];
  pvten_inc[9] += pvten_tmp[9];

 if(ifirst == 0){
   constr_cell_mat(iperd,hmat_cons_typ,hmat_int_typ,pvten_tmp);
   for(i=1;i<=9;i++){      
     fgmat_p[i] += pvten_tmp[i];
     vgmat_g[i] += (pvten_tmp[i]*roll_scg*0.5*dt/mass_hm);
   }/*endfor*/
 }/*endif*/
/* free locally assigned memory */
 if(ngrp > 0){
   free_dvector(p11,1,ngrp);
   free_dvector(p12,1,ngrp);
   free_dvector(p13,1,ngrp);
   free_dvector(p22,1,ngrp);
   free_dvector(p23,1,ngrp);
   free_dvector(p33,1,ngrp);

   free(ind1); free(ind2); free(ind3);

   free_dmatrix(x,1,3,1,ngrp);
   free_dmatrix(y,1,3,1,ngrp);
   free_dmatrix(z,1,3,1,ngrp);

   free_dmatrix(vx,1,3,1,ngrp);
   free_dmatrix(vy,1,3,1,ngrp);
   free_dmatrix(vz,1,3,1,ngrp);

   free_dvector(rm1,1,ngrp);
   free_dvector(rm2,1,ngrp);
   free_dvector(rm3,1,ngrp);

   free_dvector(dx12,1,ngrp);
   free_dvector(dy12,1,ngrp);
   free_dvector(dz12,1,ngrp);

   free_dvector(dx13,1,ngrp);
   free_dvector(dy13,1,ngrp);
   free_dvector(dz13,1,ngrp);

   free_dmatrix(xlam,1,2,1,ngrp);
 }/*endif*/
/*=======================================================================*/
  } /* end routine */
/*=======================================================================*/
