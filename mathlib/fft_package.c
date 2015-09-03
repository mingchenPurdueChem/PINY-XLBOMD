/*==========================================================================*/
/*cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc*/
/*==========================================================================*/
/*                                                                          */
/*                         PI_MD:                                           */
/*             The future of simulation technology                          */
/*             ------------------------------------                         */
/*                     Module: fft_package.c                                */
/*                                                                          */
/* 3D FFT modules                                                           */
/*                                                                          */
/*==========================================================================*/
/*cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc*/
/*==========================================================================*/



#include "standard_include.h"
#include "../typ_defs/typedefs_gen.h"
#include "../typ_defs/typedefs_cp.h"
#include "../typ_defs/typedefs_class.h"
#include "../proto_defs/proto_math.h"
#include "../proto_defs/proto_friend_lib_entry.h"
#include "../proto_defs/proto_energy_cp_local.h"
#include "../proto_defs/proto_communicate_wrappers.h"

#ifdef FFTW3
#include "fftw3.h"
#endif

#define BLOCK_PACK
#define DALLTOALL
#define SALLTOALL
#define POST_MAP
#define FFT
#define TRANS
#define DEBUG_PME_OFF

/*==========================================================================*/
/*cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc*/
/*==========================================================================*/
/*   3D FFT from sphere cut g space to full r space */
/*==========================================================================*/

void para_fft_gen3d_fwd_to_r(double *zfft, double *zfft_tmp,
                             PARA_FFT_PKG3D *para_fft_pkg3d)

/*=======================================================================*/
/*            Begin subprogram:                                          */
{/*begin routine*/
/*=======================================================================*/
/*          Local variable declarations                                  */
#include "../typ_defs/typ_mask.h"

 int iopt = 1,incl=1;
 int ier  = 0;
 int incn;
 int ndata;
 int i,j,k,m,ioff,joff,koff,kap,kbp,kcp;
 int iproc,iii,ka,ka_str,ka_end;
 int jb;
 int kb,kb_str,kb_end;
 int kc,kc_str,kc_end;
 int kstr,kend;
 int skc_fft_ka_now,ekc_fft_ka_now;
 int skb_fft_ka_now,ekb_fft_ka_now;
 int skc_fft_kb_now,ekc_fft_kb_now;
 int kadd,madd;

/*-----------------------------------------------------------------------*/
/*          Local pointers                                               */


/* Communicator */
 MPI_Comm comm = para_fft_pkg3d->comm;
 int num_proc  = para_fft_pkg3d->num_proc;
 int myid      = para_fft_pkg3d->myid; 
 int myidp1    = para_fft_pkg3d->myid+1; 

/* General Info */
 int ncoef_proc   = para_fft_pkg3d->ncoef_proc; 
 int ncoef_use    = para_fft_pkg3d->ncoef_use; 
 int igeneric_opt = para_fft_pkg3d->igeneric_opt;
 int nwork1       = para_fft_pkg3d->nwork1;
 int nwork2       = para_fft_pkg3d->nwork2;

/* FFT along kc  */
 int nkf3                = para_fft_pkg3d->nkf3; 
 int *sendcounts_fft_kc  = para_fft_pkg3d->sendcounts_fft_kc;
 int *senddspls_fft_kc   = para_fft_pkg3d->senddspls_fft_kc;
 int *recvcounts_fft_kc  = para_fft_pkg3d->recvcounts_fft_kc;
 int *recvdspls_fft_kc   = para_fft_pkg3d->recvdspls_fft_kc;
 int nfft_kc_proc        = para_fft_pkg3d->nfft_kc_proc;
 int nka_fft_kc          = para_fft_pkg3d->nka_fft_kc;
 int str_fft_kc_proc     = para_fft_pkg3d->str_fft_kc_proc;
 int end_fft_kc_proc     = para_fft_pkg3d->end_fft_kc_proc;
 int *sum_fft_kc_proc    = para_fft_pkg3d->sum_fft_kc_proc;
 int *ka_fft_kc          = para_fft_pkg3d->ka_fft_kc;
 int *kb_fft_kc          = para_fft_pkg3d->kb_fft_kc;
 int ska_fft_kc_proc     = para_fft_pkg3d->ska_fft_kc_proc;
 int eka_fft_kc_proc     = para_fft_pkg3d->eka_fft_kc_proc;
 int skb_fft_kc_proc     = para_fft_pkg3d->skb_fft_kc_proc;
 int ekb_fft_kc_proc     = para_fft_pkg3d->ekb_fft_kc_proc;
 int *nkb_fft_kc         = para_fft_pkg3d->nkb_fft_kc;
 int *ka_fft_kc_red      = para_fft_pkg3d->ka_fft_kc_red;
 int *ifax_c_f           = para_fft_pkg3d->ifax_c_f;
 double *work_1c_f       = para_fft_pkg3d->work_1c_f;
 double *work_2c_f       = para_fft_pkg3d->work_2c_f;

/* FFT along kb  */
 int nkf2                 = para_fft_pkg3d->nkf2; 
 int *sendcounts_fft_kb   = para_fft_pkg3d->sendcounts_fft_kb;
 int *senddspls_fft_kb    = para_fft_pkg3d->senddspls_fft_kb;
 int *recvcounts_fft_kb   = para_fft_pkg3d->recvcounts_fft_kb;
 int *recvdspls_fft_kb    = para_fft_pkg3d->recvdspls_fft_kb;
 int nfft_kb_proc         = para_fft_pkg3d->nfft_kb_proc;
 int str_fft_kb_proc      = para_fft_pkg3d->str_fft_kb_proc;
 int end_fft_kb_proc      = para_fft_pkg3d->end_fft_kb_proc;
 int ska_fft_kb_proc      = para_fft_pkg3d->ska_fft_kb_proc;
 int eka_fft_kb_proc      = para_fft_pkg3d->eka_fft_kb_proc;
 int skc_fft_kb_proc      = para_fft_pkg3d->skc_fft_kb_proc;
 int ekc_fft_kb_proc      = para_fft_pkg3d->ekc_fft_kb_proc;
 int *ska_fft_kb_proc_all = para_fft_pkg3d->ska_fft_kb_proc_all;
 int *eka_fft_kb_proc_all = para_fft_pkg3d->eka_fft_kb_proc_all;
 int *skc_fft_kb_proc_all = para_fft_pkg3d->skc_fft_kb_proc_all;
 int *ekc_fft_kb_proc_all = para_fft_pkg3d->ekc_fft_kb_proc_all;
 int *sum_fft_kb_proc     = para_fft_pkg3d->sum_fft_kb_proc;
 int *ifax_b_f            = para_fft_pkg3d->ifax_b_f;
 double *work_1b_f       = para_fft_pkg3d->work_1b_f;
 double *work_2b_f       = para_fft_pkg3d->work_2b_f;

/* FFT along ka  */
 int nkf1                 = para_fft_pkg3d->nkf1; 
 int *sendcounts_fft_ka   = para_fft_pkg3d->sendcounts_fft_ka;
 int *senddspls_fft_ka    = para_fft_pkg3d->senddspls_fft_ka;
 int *recvcounts_fft_ka   = para_fft_pkg3d->recvcounts_fft_ka;
 int *recvdspls_fft_ka    = para_fft_pkg3d->recvdspls_fft_ka;
 int nfft_ka_proc         = para_fft_pkg3d->nfft_ka_proc;
 int str_fft_ka_proc      = para_fft_pkg3d->str_fft_ka_proc;
 int end_fft_ka_proc      = para_fft_pkg3d->end_fft_ka_proc;
 int skb_fft_ka_proc      = para_fft_pkg3d->skb_fft_ka_proc;
 int ekb_fft_ka_proc      = para_fft_pkg3d->ekb_fft_ka_proc;
 int skc_fft_ka_proc      = para_fft_pkg3d->skc_fft_ka_proc;
 int ekc_fft_ka_proc      = para_fft_pkg3d->ekc_fft_ka_proc;
 int *skb_fft_ka_proc_all = para_fft_pkg3d->skb_fft_ka_proc_all;
 int *ekb_fft_ka_proc_all = para_fft_pkg3d->ekb_fft_ka_proc_all;
 int *skc_fft_ka_proc_all = para_fft_pkg3d->skc_fft_ka_proc_all;
 int *ekc_fft_ka_proc_all = para_fft_pkg3d->ekc_fft_ka_proc_all;
 int *ifax_a_f            = para_fft_pkg3d->ifax_a_f;
 double *work_1a_f       = para_fft_pkg3d->work_1a_f;
 double *work_2a_f       = para_fft_pkg3d->work_2a_f;


#ifdef FFTW3
 /* FFTW3 */
 fftw_plan plan_fa = para_fft_pkg3d->plan_fftw_fa;
 fftw_plan plan_fb = para_fft_pkg3d->plan_fftw_fb;
 fftw_plan plan_fc = para_fft_pkg3d->plan_fftw_fc;
#endif

/* MAPS */
 int *map_proc_post       = para_fft_pkg3d->map_proc_post;

/*==========================================================================*/
/* Transpose I  */

  if(num_proc>1){

    /*-----------------------------*/
    /* Communicate blocks          */
    /*-----------------------------*/

#ifdef SALLTOALL
    Alltoallv(&zfft[1],&(sendcounts_fft_kc[1]),&(senddspls_fft_kc[1]),
              MPI_DOUBLE,&zfft_tmp[1],&(recvcounts_fft_kc[1]),
              &(recvdspls_fft_kc[1]),MPI_DOUBLE,comm);
#endif

    /*-----------------------------*/
    /* Index Transpose plus spread */
    /*-----------------------------*/

#ifdef POST_MAP
    ndata = 2*nfft_kc_proc*nkf3; 
    for(i=1;i<=ndata;i++){zfft[i]=0.0;}

    ndata = recvdspls_fft_kc[num_proc]+recvcounts_fft_kc[num_proc];
    for(i=1;i<=ndata;i++){
     zfft[map_proc_post[i]]=zfft_tmp[i];
    }/*endfor*/
#endif

  }/*endif*/

/*==========================================================================*/
/* Perform FFT along ``c'' using zfft */

  incn = nkf3;
#ifdef FFT
#ifdef FFTW3
 /* forward ``c'' */
 if(igeneric_opt==0) {
   fftw_execute_dft(plan_fc,&(zfft[1]),&(zfft[1]));
 } else {
   fft_gen1d(zfft,nkf3,incl,nfft_kc_proc,incn,iopt,&ier,
            work_1c_f,nwork1,work_2c_f,nwork2,ifax_c_f,igeneric_opt);
 }

#else

  fft_gen1d(zfft,nkf3,incl,nfft_kc_proc,incn,iopt,&ier,
            work_1c_f,nwork1,work_2c_f,nwork2,ifax_c_f,igeneric_opt);
#endif
#endif

/*==========================================================================*/
/* Transpose II */


  if(num_proc>1){

   /*---------------------------------------------*/
   /* In place-block transpose                     */
   /*---------------------------------------------*/
   /*   i)Loop over procs=iproc                                     */
   /*  ii)Loop over the ka range to send to iproc                   */
   /* iii)Loop over the kb range you have (iproc wants it all)      */
   /*  iv)Loop over the kc range to send to iproc (you have it all) */
   /*   v)Store allowed/in range data                               */

#ifdef BLOCK_PACK
   j = 0;
   for(iproc=1;iproc<=num_proc;iproc++){
     if(ekc_fft_kb_proc_all[iproc]>skc_fft_kb_proc_all[iproc]){
       ka_str = ska_fft_kc_proc;
       ka_end = eka_fft_kc_proc;
     }else{
       ka_str = MAX(ska_fft_kb_proc_all[iproc],ska_fft_kc_proc);
       ka_end = MIN(eka_fft_kb_proc_all[iproc],eka_fft_kc_proc);
     }/*endif*/
     for(ka = ka_str;ka<= ka_end;ka++){
       kb_str = (ka==ska_fft_kc_proc ? skb_fft_kc_proc : 1);
       kb_end = (ka==eka_fft_kc_proc ? ekb_fft_kc_proc : nkb_fft_kc[ka]);
       kc_str = (ka < ska_fft_kb_proc_all[iproc] ? 
                                               skc_fft_kb_proc_all[iproc]+1
                                             : skc_fft_kb_proc_all[iproc]);
       kc_end = (ka > eka_fft_kb_proc_all[iproc] ? 
                                               ekc_fft_kb_proc_all[iproc]-1
                                             : ekc_fft_kb_proc_all[iproc]);
       i = 2*sum_fft_kc_proc[ka]*nkf3;
       for(kb=kb_str;kb<=kb_end;kb++){
         ioff   = i+2*(kc_str-1);
         for(kc=1;kc<=2*(kc_end-kc_str+1);kc++){
           zfft_tmp[(j+kc)] = zfft[(kc+ioff)];
         }/*endfor:kc*/
         iii = 2*MAX(kc_end-kc_str+1,0);
         j+= iii; i+= 2*nkf3; 
       }/*endfor:kb*/
     }/*endfor:ka */
   }/*endfor:iproc */
#endif
  /*-----------------------------*/
  /* Communicate Blocks          */
  /*-----------------------------*/

#ifdef SALLTOALL
    Alltoallv(&zfft_tmp[1],&(sendcounts_fft_kb[1]),&(senddspls_fft_kb[1]),
              MPI_DOUBLE,&zfft[1],&(recvcounts_fft_kb[1]),
              &(recvdspls_fft_kb[1]),MPI_DOUBLE,comm);
#endif
  }/*endif*/

  /*-----------------------------*/
  /* Index Transpose plus spread */
  /*-----------------------------*/

#ifdef TRANS
   ndata = 2*nfft_kb_proc*nkf2;
   for(i=1;i<=ndata;i++){zfft_tmp[i]=0.0;}
  
   if(ekc_fft_kb_proc>skc_fft_kb_proc){
      ka_str = 1;
      ka_end = nka_fft_kc;
      kadd   = 2*nka_fft_kc*nkf2;
      madd   = 2*(nka_fft_kc-ska_fft_kb_proc+1)*nkf2;
   }else{
      ka_str = ska_fft_kb_proc;
      ka_end = eka_fft_kb_proc;
      kadd   = 0;
      madd   = 0;
   }/*endif*/
   jb=0;for(j=1;j<=ka_str-1;j++){jb+=nkb_fft_kc[j];}
   i = 0; 
   for(ka = ka_str;ka<= ka_end;ka++){
     for(kb=1;kb<=nkb_fft_kc[ka];kb++){
       kc_str = skc_fft_kb_proc+1;
       kc_end = (ka>eka_fft_kb_proc ? ekc_fft_kb_proc-1 : ekc_fft_kb_proc);
       jb++;ioff = 2*kb_fft_kc[jb] - 1;
       if(ka>=ska_fft_kb_proc){
         joff = ioff+ 2*(ka-ska_fft_kb_proc)*nkf2;
         zfft_tmp[(joff)]   = zfft[(i+1)];
         zfft_tmp[(joff+1)] = zfft[(i+2)];
         i+= 2; 
       }/*endfor*/
       ioff += (madd + 2*(ka-1)*nkf2);
       for(kc=1,m=0;kc<=2*(kc_end-kc_str+1);kc+=2,m+=kadd){
         zfft_tmp[(m+ioff)]   = zfft[(kc+i)];
         zfft_tmp[(m+ioff+1)] = zfft[(kc+i+1)];
       }/*endfor*/
       iii = MAX(kc_end-kc_str+1,0); i += 2*iii;
     }/*endfor*/
   }/*endfor*/

#endif

/*==========================================================================*/
/* Perform FFT along ``b'' using zfft_tmp */

  incn = nkf2;

#ifdef FFT
#ifdef FFTW3
  /* forward ``b'' */
 if(igeneric_opt==0) {
 fftw_execute_dft(plan_fb,&(zfft_tmp[1]),&(zfft_tmp[1]));
 } else {
   fft_gen1d(zfft_tmp,nkf2,incl,nfft_kb_proc,incn,iopt,&ier,
            work_1b_f,nwork1,work_2b_f,nwork2,ifax_b_f,igeneric_opt);
 }

#else
  fft_gen1d(zfft_tmp,nkf2,incl,nfft_kb_proc,incn,iopt,&ier,
            work_1b_f,nwork1,work_2b_f,nwork2,ifax_b_f,igeneric_opt);
#endif
#endif

/*==========================================================================*/
/* Transpose III */


 if(num_proc>1){

#ifdef BLOCK_PACK
   /*---------------------------------------------*/
   /* In place-block transpose                     */
   /*---------------------------------------------*/
   /*  i)Loop over procs=iproc                                        */
   /*  i)Loop over the kc range to send to iproc                      */
   /*  i)Loop over ka's you have (iproc wants them all)               */
   /*  iv)Loop over the kb range to send to iproc (you have them all) */
   /*   v)Store allowed/in range data                                 */

   j=0; 
   kadd = 2*(nka_fft_kc-ska_fft_kb_proc+1)*nkf2; 
   madd = 2*nka_fft_kc*nkf2;
   for(iproc=1;iproc<=num_proc;iproc++){
     kc_str = MAX(skc_fft_kb_proc,skc_fft_ka_proc_all[iproc]);
     kc_end = MIN(ekc_fft_kb_proc,ekc_fft_ka_proc_all[iproc]);
     i = (kc_str==skc_fft_kb_proc ?  0 : 
                         kadd + (kc_str-skc_fft_kb_proc-1)*madd);
     for(kc=kc_str;kc<=kc_end;kc++){
       ka_str = (kc==skc_fft_kb_proc ? ska_fft_kb_proc : 1);
       ka_end = (kc==ekc_fft_kb_proc ? eka_fft_kb_proc : nka_fft_kc);
       kb_str = (kc==skc_fft_ka_proc_all[iproc] ? 
                                       skb_fft_ka_proc_all[iproc]:1);
       kb_end = (kc==ekc_fft_ka_proc_all[iproc] ? 
                                       ekb_fft_ka_proc_all[iproc]:nkf2);
       for(ka=ka_str;ka<=ka_end;ka++){
         ioff = i + 2*kb_str - 2;
         for(kb=1;kb<=2*(kb_end-kb_str+1);kb++){
           zfft[(j+kb)] = zfft_tmp[(ioff+kb)];
         }/*endfor*/
         iii = 2*MAX(kb_end-kb_str+1,0);
         j+=iii;i+= 2*nkf2;
       }/*endfor: ka*/
     }/*endfor: kc*/ 
   }/*endfor: iproc*/
#endif

    /*-----------------------------*/
    /* Communicate Blocks          */
    /*-----------------------------*/

#ifdef DALLTOALL
    Alltoallv(&zfft[1],&(sendcounts_fft_ka[1]),&(senddspls_fft_ka[1]),
              MPI_DOUBLE,&zfft_tmp[1],&(recvcounts_fft_ka[1]),
              &(recvdspls_fft_ka[1]),MPI_DOUBLE,comm);
#endif

 }/*endif*/

#ifdef TRANS
  /*-----------------------------*/
  /* Index Transpose plus spread */
  /*-----------------------------*/

   ndata = 2*nfft_ka_proc*nkf1;

   for(i=1;i<=ndata;i++){zfft[i]=0.0;}

   i = 0; 
   madd = 2*(nkf2-skb_fft_ka_proc+1)*nkf1;
   for(kc=skc_fft_ka_proc;kc<=ekc_fft_ka_proc;kc++){
     kb_str = (kc==skc_fft_ka_proc ? skb_fft_ka_proc : 1);
     kb_end = (kc==ekc_fft_ka_proc ? ekb_fft_ka_proc : nkf2);
     kadd   = (kc==skc_fft_ka_proc ? 0 : 
                       madd + 2*(kc-skc_fft_ka_proc-1)*nkf1*nkf2);
     for(ka=1;ka<=nka_fft_kc;ka++){
       ioff   = kadd + 2*ka_fft_kc_red[ka] - 1 ;
       for(k=1,m=0;k<=2*(kb_end-kb_str+1);k+=2,m+=2*nkf1){
        zfft[(m+ioff)]   = zfft_tmp[(i+k)];
        zfft[(m+ioff+1)] = zfft_tmp[(i+k+1)];
       }/*endfor:kb*/
       iii = MAX(kb_end-kb_str+1,0);i += 2*iii;
     }/*endfor:ka*/
   }/*endfor:kc*/

#endif

/*==========================================================================*/
/* Perform FFT along ``a'' using zfft                                       */

  incn = nkf1;

#ifdef FFT
#ifdef FFTW3
  /* forward ``a'' */
 if(igeneric_opt==0) {
 fftw_execute_dft(plan_fa,&(zfft[1]),&(zfft[1]));
 } else {
    fft_gen1d(zfft,nkf1,incl,nfft_ka_proc,incn,iopt,&ier,
            work_1a_f,nwork1,work_2a_f,nwork2,ifax_a_f,igeneric_opt);
 }
#else
  fft_gen1d(zfft,nkf1,incl,nfft_ka_proc,incn,iopt,&ier,
            work_1a_f,nwork1,work_2a_f,nwork2,ifax_a_f,igeneric_opt);
#endif
#endif
/*-----------------------------------------------------------------------*/
   }/*end routine*/ 
/*==========================================================================*/






/*==========================================================================*/
/*cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc*/
/*==========================================================================*/
/*   Sngl pack the coefs for 3D FFT */
/*==========================================================================*/

void sngl_pack_coef(double *cre,double *cim,double *zfft,
                    PARA_FFT_PKG3D *para_fft_pkg3d)

/*=======================================================================*/
/*            Begin subprogram:                                          */
   {/*begin routine*/
/*=======================================================================*/
/*          Local variable declarations                                  */

   int i;
   int ndata       = para_fft_pkg3d->ndata_kc;
   int num_proc    = para_fft_pkg3d->num_proc;
   int ncoef_use   = para_fft_pkg3d->ncoef_use;
   int ncoef_proc  = para_fft_pkg3d->ncoef_proc;
   int *map_proc   = para_fft_pkg3d->map_proc;
   int *map_c_proc = para_fft_pkg3d->map_c_proc;

/*=========================================================================*/
/* Pack the data up: top and bottom half of k-space : zero fill in scalar */

  if(num_proc==1){for(i=1;i<=ndata;i++){zfft[i]=0.0;} }

  for(i=1;i<=ncoef_use;i++){
    zfft[(map_proc[i])]     =  cre[i];
    zfft[(map_proc[i]+1)]   =  cim[i];
    zfft[(map_c_proc[i])]   =  cre[i]; 
    zfft[(map_c_proc[i]+1)] = -cim[i];
  }/*endfor*/

  if(ncoef_proc > ncoef_use){
    i = ncoef_proc;
    zfft[(map_proc[i])]     = cre[i];
    zfft[(map_proc[i]+1)]   = cim[i];
  }/*endif*/


/*-----------------------------------------------------------------------*/
   }/*end routine*/ 
/*==========================================================================*/



/*==========================================================================*/
/*cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc*/
/*==========================================================================*/
/*   Sngl pack the coefs for 3D FFT */
/*==========================================================================*/

void pme_sngl_pack_coef(double *cre,double *cim,double *zfft,
                        PARA_FFT_PKG3D *para_fft_pkg3d)

/*=======================================================================*/
/*            Begin subprogram:                                          */
   {/*begin routine*/
/*=======================================================================*/
/*          Local variable declarations                                  */

   int i;
   int ndata       = para_fft_pkg3d->ndata_kc;
   int num_proc    = para_fft_pkg3d->num_proc;
   int ncoef_use   = para_fft_pkg3d->ncoef_use;
   int ncoef_proc  = para_fft_pkg3d->ncoef_proc;
   int *map_proc   = para_fft_pkg3d->map_proc;
   int *map_c_proc = para_fft_pkg3d->map_c_proc;

/*=========================================================================*/
/* Pack the data up: top and bottom half of k-space : zero fill in scalar */

  if(num_proc==1){for(i=1;i<=ndata;i++){zfft[i]=0.0;} }

  for(i=1;i<=ncoef_use;i++){
    zfft[(map_proc[i])]     =  cre[i];
    zfft[(map_proc[i]+1)]   =  cim[i];
    zfft[(map_c_proc[i])]   =  cre[i]; 
    zfft[(map_c_proc[i]+1)] = -cim[i];
  }/*endfor*/

  if(ncoef_proc > ncoef_use){
    i = ncoef_proc;
    zfft[(map_proc[i])]     =  cre[i];
    zfft[(map_proc[i]+1)]   =  cim[i];
  }/*endif*/


/*-----------------------------------------------------------------------*/
   }/*end routine*/ 
/*==========================================================================*/



/*==========================================================================*/
/*cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc*/
/*==========================================================================*/
/*   Dble pack the coefs for 3D FFT */
/*==========================================================================*/

void dble_pack_coef(double *c1re, double *c1im,double *c2re, double *c2im,
                    double *zfft,PARA_FFT_PKG3D *para_fft_pkg3d)

/*=======================================================================*/
/*            Begin subprogram:                                          */
   {/*begin routine*/
/*=======================================================================*/
/*          Local variable declarations                                  */
   int i;
   int ndata       = para_fft_pkg3d->ndata_kc;
   int num_proc    = para_fft_pkg3d->num_proc;
   int ncoef_use   = para_fft_pkg3d->ncoef_use;
   int ncoef_proc  = para_fft_pkg3d->ncoef_proc;
   int *map_proc   = para_fft_pkg3d->map_proc;
   int *map_c_proc = para_fft_pkg3d->map_c_proc;

/*=========================================================================*/
/* Pack the data up: top and bottom half of k-space : zero fill in scalar */

  if(num_proc==1){for(i=1;i<=ndata;i++){zfft[i]=0.0;} }

  for(i=1;i<=ncoef_use;i++){
    zfft[(map_proc[i])]     =   c1re[i] - c2im[i];
    zfft[(map_proc[i]+1)]   =   c1im[i] + c2re[i];
    zfft[(map_c_proc[i])]   =   c1re[i] + c2im[i]; 
    zfft[(map_c_proc[i]+1)] =  -c1im[i] + c2re[i];
  }/*endfor*/

  if(ncoef_proc > ncoef_use){
    i = ncoef_proc;
    zfft[(map_proc[i])]     = c1re[i];
    zfft[(map_proc[i]+1)]   = c2re[i];
  }/*endif*/


/*-----------------------------------------------------------------------*/
   }/*end routine*/ 
/*==========================================================================*/




/*==========================================================================*/
/*cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc*/
/*==========================================================================*/
/*  3D FFT from full r-space to sphere cut g-space  */
/*==========================================================================*/

void para_fft_gen3d_bck_to_g(double *zfft, double *zfft_tmp,
                             PARA_FFT_PKG3D *para_fft_pkg3d)

/*=======================================================================*/
/*            Begin subprogram:                                          */
   {/*begin routine*/
/*=======================================================================*/
/*          Local variable declarations                                  */
#include "../typ_defs/typ_mask.h"

 int iopt = -1,incl=1;
 int ier  = 0;
 int incn;
 int i,j,k,m,ka,kb,kc,ioff,joff,koff,iii;
 int kap,kbp,kcp,kstr,kend,jb;
 int skc_fft_kb_now,ekc_fft_kb_now;
 int skb_fft_kb_now,ekb_fft_kb_now;
 int skb_fft_ka_now,ekb_fft_ka_now;
 int ndata,iproc;
 int kc_str,kc_end,kb_str,kb_end,ka_str,ka_end;
 double scale;
 int nfft;
 int kadd,madd;

/*-----------------------------------------------------------------------*/
/*          Local pointers                                               */

/* Communicator */
 MPI_Comm comm = para_fft_pkg3d->comm;
 int num_proc  = para_fft_pkg3d->num_proc;
 int myid      = para_fft_pkg3d->myid; 
 int myidp1    = para_fft_pkg3d->myid+1; 

/* General Info */
 int nktot        = para_fft_pkg3d->nktot;
 int ncoef        = para_fft_pkg3d->ncoef;
 int ncoef_proc   = para_fft_pkg3d->ncoef_proc;
 int ncoef_use    = para_fft_pkg3d->ncoef_use;
 int icoef_off    = para_fft_pkg3d->icoef_off;
 int icoef_strt   = para_fft_pkg3d->icoef_strt;
 int igeneric_opt = para_fft_pkg3d->igeneric_opt;
 int scale_opt    = para_fft_pkg3d->scale_opt; 
 int nwork1       = para_fft_pkg3d->nwork1;
 int nwork2       = para_fft_pkg3d->nwork2;

/* Map */
 int *map_proc_post = para_fft_pkg3d->map_proc_post; 

/* FFT along kc  */
 int nkf3                = para_fft_pkg3d->nkf3; 
 int *sendcounts_fft_kc  = para_fft_pkg3d->sendcounts_fft_kc;
 int *senddspls_fft_kc   = para_fft_pkg3d->senddspls_fft_kc;
 int *recvcounts_fft_kc  = para_fft_pkg3d->recvcounts_fft_kc;
 int *recvdspls_fft_kc   = para_fft_pkg3d->recvdspls_fft_kc;
 int nfft_kc_proc        = para_fft_pkg3d->nfft_kc_proc;
 int str_fft_kc_proc     = para_fft_pkg3d->str_fft_kc_proc;
 int end_fft_kc_proc     = para_fft_pkg3d->end_fft_kc_proc;
 int nka_fft_kc          = para_fft_pkg3d->nka_fft_kc;
 int *sum_fft_kc_proc    = para_fft_pkg3d->sum_fft_kc_proc;
 int *nkb_fft_kc         = para_fft_pkg3d->nkb_fft_kc;
 int *ka_fft_kc          = para_fft_pkg3d->ka_fft_kc;
 int *ka_fft_kc_red      = para_fft_pkg3d->ka_fft_kc_red;
 int *kb_fft_kc          = para_fft_pkg3d->kb_fft_kc;
 int ska_fft_kc_proc     = para_fft_pkg3d->ska_fft_kc_proc;
 int eka_fft_kc_proc     = para_fft_pkg3d->eka_fft_kc_proc;
 int skb_fft_kc_proc     = para_fft_pkg3d->skb_fft_kc_proc;
 int ekb_fft_kc_proc     = para_fft_pkg3d->ekb_fft_kc_proc;
 int *ifax_c_r           = para_fft_pkg3d->ifax_c_r;
 double *work_1c_r       = para_fft_pkg3d->work_1c_r;
 double *work_2c_r       = para_fft_pkg3d->work_2c_r;

#ifdef FFTW3
  fftw_plan plan_ba = para_fft_pkg3d->plan_fftw_ba;
  fftw_plan plan_bb = para_fft_pkg3d->plan_fftw_bb;
  fftw_plan plan_bc = para_fft_pkg3d->plan_fftw_bc;
#endif

/* FFT along kb  */
 int nkf2                 = para_fft_pkg3d->nkf2; 
 int *sendcounts_fft_kb   = para_fft_pkg3d->sendcounts_fft_kb;
 int *senddspls_fft_kb    = para_fft_pkg3d->senddspls_fft_kb;
 int *recvcounts_fft_kb   = para_fft_pkg3d->recvcounts_fft_kb;
 int *recvdspls_fft_kb    = para_fft_pkg3d->recvdspls_fft_kb;
 int nfft_kb_proc         = para_fft_pkg3d->nfft_kb_proc;
 int str_fft_kb_proc      = para_fft_pkg3d->str_fft_kb_proc;
 int end_fft_kb_proc      = para_fft_pkg3d->end_fft_kb_proc;
 int ska_fft_kb_proc      = para_fft_pkg3d->ska_fft_kb_proc;
 int eka_fft_kb_proc      = para_fft_pkg3d->eka_fft_kb_proc;
 int skc_fft_kb_proc      = para_fft_pkg3d->skc_fft_kb_proc;
 int ekc_fft_kb_proc      = para_fft_pkg3d->ekc_fft_kb_proc;
 int *ska_fft_kb_proc_all = para_fft_pkg3d->ska_fft_kb_proc_all;
 int *eka_fft_kb_proc_all = para_fft_pkg3d->eka_fft_kb_proc_all;
 int *skc_fft_kb_proc_all = para_fft_pkg3d->skc_fft_kb_proc_all;
 int *ekc_fft_kb_proc_all = para_fft_pkg3d->ekc_fft_kb_proc_all;
 int *sum_fft_kb_proc     = para_fft_pkg3d->sum_fft_kb_proc;
 int *ifax_b_r            = para_fft_pkg3d->ifax_b_r;
 double *work_1b_r        = para_fft_pkg3d->work_1b_r;
 double *work_2b_r        = para_fft_pkg3d->work_2b_r;

/* FFT along ka  */
 int nkf1                 = para_fft_pkg3d->nkf1; 
 int *sendcounts_fft_ka   = para_fft_pkg3d->sendcounts_fft_ka;
 int *senddspls_fft_ka    = para_fft_pkg3d->senddspls_fft_ka;
 int *recvcounts_fft_ka   = para_fft_pkg3d->recvcounts_fft_ka;
 int *recvdspls_fft_ka    = para_fft_pkg3d->recvdspls_fft_ka;
 int nfft_ka_proc         = para_fft_pkg3d->nfft_ka_proc;
 int str_fft_ka_proc      = para_fft_pkg3d->str_fft_ka_proc;
 int end_fft_ka_proc      = para_fft_pkg3d->end_fft_ka_proc;
 int skb_fft_ka_proc      = para_fft_pkg3d->skb_fft_ka_proc;
 int ekb_fft_ka_proc      = para_fft_pkg3d->ekb_fft_ka_proc;
 int skc_fft_ka_proc      = para_fft_pkg3d->skc_fft_ka_proc;
 int ekc_fft_ka_proc      = para_fft_pkg3d->ekc_fft_ka_proc;
 int *skb_fft_ka_proc_all = para_fft_pkg3d->skb_fft_ka_proc_all;
 int *ekb_fft_ka_proc_all = para_fft_pkg3d->ekb_fft_ka_proc_all;
 int *skc_fft_ka_proc_all = para_fft_pkg3d->skc_fft_ka_proc_all;
 int *ekc_fft_ka_proc_all = para_fft_pkg3d->ekc_fft_ka_proc_all;
 int *ifax_a_r            = para_fft_pkg3d->ifax_a_r;
 double *work_1a_r        = para_fft_pkg3d->work_1a_r;
 double *work_2a_r        = para_fft_pkg3d->work_2a_r;

/*==========================================================================*/
/* Perform Inverse FFT along ``a'' using zfft                               */

  incn = nkf1;
#ifdef FFT
#ifdef FFTW3
  /* backward ``a'' */
 if(igeneric_opt==0) {
   fftw_execute_dft(plan_ba,&(zfft[1]),&(zfft[1]));
 } else {
   fft_gen1d(zfft,nkf1,incl,nfft_ka_proc,incn,iopt,&ier,
            work_1a_r,nwork1,work_2a_r,nwork2,ifax_a_r,igeneric_opt);
 }
#else
  fft_gen1d(zfft,nkf1,incl,nfft_ka_proc,incn,iopt,&ier,
            work_1a_r,nwork1,work_2a_r,nwork2,ifax_a_r,igeneric_opt);
#endif
#endif
/*==========================================================================*/
/*  Inverse Transpose III: Back to the coefs                                */

#ifdef TRANS
  /*---------------------------------------*/
  /* Index inverse transpose plus contract */
  /*---------------------------------------*/

   i = 0; 
   madd = 2*(nkf2-skb_fft_ka_proc+1)*nkf1;
   for(kc=skc_fft_ka_proc;kc<=ekc_fft_ka_proc;kc++){
     kb_str = (kc==skc_fft_ka_proc ? skb_fft_ka_proc : 1);
     kb_end = (kc==ekc_fft_ka_proc ? ekb_fft_ka_proc : nkf2);
     kadd   = (kc==skc_fft_ka_proc ? 0 : 
                       madd + 2*(kc-skc_fft_ka_proc-1)*nkf1*nkf2);
     for(ka=1;ka<=nka_fft_kc;ka++){
       ioff   = kadd + 2*ka_fft_kc_red[ka] - 1 ;
       for(k=1,m=0;k<=2*(kb_end-kb_str+1);k+=2,m+=2*nkf1){
         zfft_tmp[(i+k)]   = zfft[(m+ioff)];
         zfft_tmp[(i+k+1)] = zfft[(m+ioff+1)] ;
       }/*endfor:kb*/
       iii = MAX(kb_end-kb_str+1,0);i += 2*iii;
     }/*endfor:ka*/
   }/*endfor:kc*/

#endif

 if(num_proc>1){

    /*-----------------------------*/
    /* Communicate Blocks          */
    /*-----------------------------*/

#ifdef DALLTOALL
    Alltoallv(&zfft_tmp[1],&(recvcounts_fft_ka[1]),&(recvdspls_fft_ka[1]),
              MPI_DOUBLE,&zfft[1],&(sendcounts_fft_ka[1]),
              &(senddspls_fft_ka[1]),MPI_DOUBLE,comm);
#endif

#ifdef BLOCK_PACK

   /*---------------------------------------------*/
   /* In place-block inverse transpose            */
   /*---------------------------------------------*/
   /*   0) Alltoallv                                                 */
   /*   i)Loop over procs=iproc                                      */
   /*   ii)Loop over kc's you have                                   */
   /* iii)Loop over the ka range to send to iproc                   */
   /*  iv)Loop over the kb range to send to iproc */
   /*   v)Store allowed/in range data            */
 
   j=0; 
   kadd = 2*(nka_fft_kc-ska_fft_kb_proc+1)*nkf2; 
   madd = 2*nka_fft_kc*nkf2;
   for(iproc=1;iproc<=num_proc;iproc++){
     kc_str = MAX(skc_fft_kb_proc,skc_fft_ka_proc_all[iproc]);
     kc_end = MIN(ekc_fft_kb_proc,ekc_fft_ka_proc_all[iproc]);
     i = (kc_str==skc_fft_kb_proc ?  0 : 
                         kadd + (kc_str-skc_fft_kb_proc-1)*madd);
     for(kc=kc_str;kc<=kc_end;kc++){
       ka_str = (kc==skc_fft_kb_proc ? ska_fft_kb_proc : 1);
       ka_end = (kc==ekc_fft_kb_proc ? eka_fft_kb_proc : nka_fft_kc);
       kb_str = (kc==skc_fft_ka_proc_all[iproc] ? 
                                       skb_fft_ka_proc_all[iproc]:1);
       kb_end = (kc==ekc_fft_ka_proc_all[iproc] ? 
                                       ekb_fft_ka_proc_all[iproc]:nkf2);
       for(ka=ka_str;ka<=ka_end;ka++){
         ioff = i + 2*kb_str - 2;
         for(kb=1;kb<=2*(kb_end-kb_str+1);kb++){
           zfft_tmp[(ioff+kb)] = zfft[(j+kb)];
         }/*endfor*/
         iii = 2*MAX(kb_end-kb_str+1,0);
         j+=iii;i+= 2*nkf2;
       }/*endfor: ka*/
     }/*endfor: kc*/ 
   }/*endfor: iproc*/

#endif

 }/*endif*/

/*==========================================================================*/
/* Perform  Inverse FFT along ``b'' using zfft_tmp                          */

  incn = nkf2;
#ifdef FFT
#ifdef FFTW3
  /* backward ``b'' */
 if(igeneric_opt==0) {
   fftw_execute_dft(plan_bb,&(zfft_tmp[1]),&(zfft_tmp[1]));
 } else {
   fft_gen1d(zfft_tmp,nkf2,incl,nfft_kb_proc,incn,iopt,&ier,
            work_1b_r,nwork1,work_2b_r,nwork2,ifax_b_r,igeneric_opt);
 }
#else
  fft_gen1d(zfft_tmp,nkf2,incl,nfft_kb_proc,incn,iopt,&ier,
            work_1b_r,nwork1,work_2b_r,nwork2,ifax_b_r,igeneric_opt);
#endif
#endif

/*==========================================================================*/
/* Inverse Transpose II */

  /*---------------------------------------*/
  /* Inverse Index Transpose plus contract */
  /*---------------------------------------*/

#ifdef TRANS
   if(ekc_fft_kb_proc>skc_fft_kb_proc){
      ka_str = 1;
      ka_end = nka_fft_kc;
      kadd   = 2*nka_fft_kc*nkf2;
      madd   = 2*(nka_fft_kc-ska_fft_kb_proc+1)*nkf2;
   }else{
      ka_str = ska_fft_kb_proc;
      ka_end = eka_fft_kb_proc;
      kadd   = 0;
      madd   = 0;
   }/*endif*/
   jb=0;for(j=1;j<=ka_str-1;j++){jb+=nkb_fft_kc[j];}
   i = 0; 
   for(ka = ka_str;ka<= ka_end;ka++){
     for(kb=1;kb<=nkb_fft_kc[ka];kb++){
       kc_str = skc_fft_kb_proc+1;
       kc_end = (ka>eka_fft_kb_proc ? ekc_fft_kb_proc-1 : ekc_fft_kb_proc);
       jb++;ioff = 2*kb_fft_kc[jb] - 1;
       if(ka>=ska_fft_kb_proc){
         joff = ioff+ 2*(ka-ska_fft_kb_proc)*nkf2;
         zfft[(i+1)]   = zfft_tmp[(joff)];
         zfft[(i+2)]   = zfft_tmp[(joff+1)];
         i+= 2; 
       }/*endfor*/
       ioff += (madd + 2*(ka-1)*nkf2);
       for(kc=1,m=0;kc<=2*(kc_end-kc_str+1);kc+=2,m+=kadd){
         zfft[(kc+i)]   = zfft_tmp[(m+ioff)];
         zfft[(kc+i+1)] = zfft_tmp[(m+ioff+1)];
       }/*endfor*/
       iii = MAX(kc_end-kc_str+1,0); i += 2*iii;
     }/*endfor*/
   }/*endfor*/
#endif

   if(num_proc>1){

    /*-----------------------------*/
    /* Communicate Blocks          */
    /*-----------------------------*/

#ifdef SALLTOALL
    Alltoallv(&zfft[1],&(recvcounts_fft_kb[1]),&(recvdspls_fft_kb[1]),
              MPI_DOUBLE,&zfft_tmp[1],&(sendcounts_fft_kb[1]),
              &(senddspls_fft_kb[1]),MPI_DOUBLE,comm);
#endif

#ifdef BLOCK_PACK
    /*---------------------------------------------*/
    /* Inverse in  place-block transpose           */
    /*---------------------------------------------*/
    /*   i)Loop over procs=iproc                   */
    /*  ii)Loop over the ka range to send to iproc */
    /* iii)Loop over the kb range you have         */
    /*  iv)Loop over the kc range to send to iproc */
    /*   v)Store allowed/in range data             */

   j = 0;
   for(iproc=1;iproc<=num_proc;iproc++){
     if(ekc_fft_kb_proc_all[iproc]>skc_fft_kb_proc_all[iproc]){
       ka_str = ska_fft_kc_proc;
       ka_end = eka_fft_kc_proc;
     }else{
       ka_str = MAX(ska_fft_kb_proc_all[iproc],ska_fft_kc_proc);
       ka_end = MIN(eka_fft_kb_proc_all[iproc],eka_fft_kc_proc);
     }/*endif*/
     for(ka = ka_str;ka<= ka_end;ka++){
       kb_str = (ka==ska_fft_kc_proc ? skb_fft_kc_proc : 1);
       kb_end = (ka==eka_fft_kc_proc ? ekb_fft_kc_proc : nkb_fft_kc[ka]);
       kc_str = (ka < ska_fft_kb_proc_all[iproc] ? 
                                               skc_fft_kb_proc_all[iproc]+1
                                             : skc_fft_kb_proc_all[iproc]);
       kc_end = (ka > eka_fft_kb_proc_all[iproc] ? 
                                               ekc_fft_kb_proc_all[iproc]-1
                                             : ekc_fft_kb_proc_all[iproc]);
       i = 2*sum_fft_kc_proc[ka]*nkf3;
       for(kb=kb_str;kb<=kb_end;kb++){
         ioff   = i+2*(kc_str-1);
         for(kc=1;kc<=2*(kc_end-kc_str+1);kc++){
           zfft[(kc+ioff)] = zfft_tmp[(j+kc)];
         }/*endfor:kc*/
         iii = 2*MAX(kc_end-kc_str+1,0);
         j+= iii; i+= 2*nkf3; 
       }/*endfor:kb*/
     }/*endfor:ka */
   }/*endfor:iproc */
#endif

   }/*endif:num_proc > 1*/

/*==========================================================================*/
/* Perform Inverse FFT along ``c'' using zfft                              */

  incn = nkf3;
#ifdef FFT
#ifdef FFTW3
  /* backward ``c'' */
 if(igeneric_opt==0) {
   fftw_execute_dft(plan_bc,&(zfft[1]),&(zfft[1]));
 } else {
   fft_gen1d(zfft,nkf3,incl,nfft_kc_proc,incn,iopt,&ier,
            work_1c_r,nwork1,work_2c_r,nwork2,ifax_c_r,igeneric_opt);
 }
#else
  fft_gen1d(zfft,nkf3,incl,nfft_kc_proc,incn,iopt,&ier,
            work_1c_r,nwork1,work_2c_r,nwork2,ifax_c_r,igeneric_opt);
#endif
#endif

/*==========================================================================*/
/*  Inverse Tranpose I:                                                     */

  if(num_proc>1){

    /*---------------------------------------*/
    /* Inverse Index Transpose plus contract */
    /*---------------------------------------*/
#ifdef POST_MAP
    ndata = recvdspls_fft_kc[num_proc]+recvcounts_fft_kc[num_proc];
    for(i=1;i<=ndata;i++){
      zfft_tmp[i]=zfft[map_proc_post[i]];
    }/*endfor*/
#endif

    /*-----------------------------*/
    /* Communicate blocks          */
    /*-----------------------------*/

#ifdef SALLTOALL
    Alltoallv(&zfft_tmp[1],&(recvcounts_fft_kc[1]),&(recvdspls_fft_kc[1]),
              MPI_DOUBLE,&zfft[1],&(sendcounts_fft_kc[1]),
              &(senddspls_fft_kc[1]),MPI_DOUBLE,comm);
#endif

  }/*endif*/
   
/*==========================================================================*/
/*   IV) scale zfft if necessary                                           */

  if(scale_opt == 1){
    ndata = 2*nfft_kc_proc*nkf3;
    nfft  = nkf1*nkf2*nkf3; scale = 1.0/((double) nfft);
    for(i=1;i<=ndata;i++){zfft[i] *= scale;}
  }/* endif */

  if(scale_opt == -1){
    ndata =  2*nfft_kc_proc*nkf3;
    nfft  = nkf1*nkf2*nkf3; scale = ((double) nfft);
    for(i=1;i<=ndata;i++){zfft[i] *= scale;}
  }/* endif */

/*-----------------------------------------------------------------------*/
   }/*end routine*/ 
/*==========================================================================*/





/*==========================================================================*/
/*cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc*/
/*==========================================================================*/
/*  Sngl unpack the coefs */
/*==========================================================================*/

void sngl_upack_coef(double *cre,double *cim,double *zfft,
                     PARA_FFT_PKG3D *para_fft_pkg3d)

/*=======================================================================*/
/*            Begin subprogram:                                          */
   {/*begin routine*/
/*=======================================================================*/
/*          Local variable declarations                                  */

 int i;
 int ncoef_proc = para_fft_pkg3d->ncoef_proc;
 int ncoef_use  = para_fft_pkg3d->ncoef_use;
 int *map_proc  = para_fft_pkg3d->map_proc;

/*=======================================================================*/
/*  Unpack the data : Top half of k space only */

  for(i=1;i<=ncoef_use;i++){
    cre[i]=zfft[map_proc[i]];
    cim[i]=zfft[(map_proc[i]+1)];
  }/*endfor*/
  if(ncoef_proc>ncoef_use){
    cim[ncoef_proc]=0.0;
    cre[ncoef_proc]=zfft[map_proc[i]];
  }/*endif*/

/*-----------------------------------------------------------------------*/
   }/*end routine*/ 
/*==========================================================================*/


/*==========================================================================*/
/*cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc*/
/*==========================================================================*/
/*  Sngl unpack the coefs */
/*==========================================================================*/

void sngl_upack_coef_sum(double *cre,double *cim,double *zfft,
                         PARA_FFT_PKG3D *para_fft_pkg3d)

/*=======================================================================*/
/*            Begin subprogram:                                          */
   {/*begin routine*/
/*=======================================================================*/
/*          Local variable declarations                                  */

 int i,ncoef_min;
 int ncoef_proc = para_fft_pkg3d->ncoef_proc;
 int ncoef_use  = para_fft_pkg3d->ncoef_use;
 int *map_proc  = para_fft_pkg3d->map_proc;

/*=======================================================================*/
/*  Unpack the data : Top half of k space only */

  ncoef_min = MIN(ncoef_proc,ncoef_use);
  for(i=1;i<=ncoef_min;i++){
    cre[i]-=(4.0*zfft[map_proc[i]]);
    cim[i]-=(4.0*zfft[(map_proc[i]+1)]);
  }/*endfor*/
  if(ncoef_proc>ncoef_use){
    i = ncoef_proc;
    cre[i]-=(2.0*zfft[map_proc[i]]);
  }/*endif*/

/*-----------------------------------------------------------------------*/
   }/*end routine*/ 
/*==========================================================================*/




/*==========================================================================*/
/*cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc*/
/*==========================================================================*/
/*  Sngl unpack the density */
/*==========================================================================*/

void sngl_upack_rho(double *zfft,double *rfft,PARA_FFT_PKG3D *para_fft_pkg3d)

/*=======================================================================*/
/*            Begin subprogram:                                          */
   {/*begin routine*/
/*=======================================================================*/
/*          Local variable declarations                                  */

 int m,i;
 int nfft_proc = para_fft_pkg3d->nfft_proc;
 int ndata     = nfft_proc/2;

/*=======================================================================*/
/*  Unpack the data : Top half of k space only */

  for(i=1,m=1;i<=ndata;i++,m+=2){
    rfft[i] = zfft[m];
  }/*endfor*/

/*-----------------------------------------------------------------------*/
   }/*end routine*/ 
/*==========================================================================*/

/*==========================================================================*/
/*cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc*/
/*==========================================================================*/
/*  Sngl unpack the density */
/*==========================================================================*/

void sngl_upack_rho_sum(double *zfft,double *rfft,
                        PARA_FFT_PKG3D *para_fft_pkg3d)

/*=======================================================================*/
/*            Begin subprogram:                                          */
   {/*begin routine*/
/*=======================================================================*/
/*          Local variable declarations                                  */

 int m,i;
 int nfft_proc = para_fft_pkg3d->nfft_proc;
 int ndata     = nfft_proc/2;

/*=======================================================================*/
/*  Unpack the data : Top half of k space only */

  for(i=1,m=1;i<=ndata;i++,m+=2){
    rfft[i] += zfft[m];
  }/*endfor*/

/*-----------------------------------------------------------------------*/
   }/*end routine*/ 
/*==========================================================================*/



/*==========================================================================*/
/*cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc*/
/*==========================================================================*/
/*  Sngl unpack the density */
/*==========================================================================*/

void sngl_upack_rho_scal(double *zfft,double *rfft,
                         PARA_FFT_PKG3D *para_fft_pkg3d,double scal)

/*=======================================================================*/
/*            Begin subprogram:                                          */
   {/*begin routine*/
/*=======================================================================*/
/*          Local variable declarations                                  */

 int m,i;
 int nfft_proc = para_fft_pkg3d->nfft_proc;
 int ndata     = nfft_proc/2;

/*=======================================================================*/
/*  Unpack the data : Top half of k space only */

  for(i=1,m=1;i<=ndata;i++,m+=2){
    rfft[i] = zfft[m]*scal;
  }/*endfor*/

/*-----------------------------------------------------------------------*/
   }/*end routine*/ 
/*==========================================================================*/


/*==========================================================================*/
/*cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc*/
/*==========================================================================*/
/*  Sngl pack the density */
/*==========================================================================*/

void sngl_pack_rho(double *zfft,double *rfft,PARA_FFT_PKG3D *para_fft_pkg3d)

/*=======================================================================*/
/*            Begin subprogram:                                          */
   {/*begin routine*/
/*=======================================================================*/
/*          Local variable declarations                                  */

 int m,i;
 int nfft_proc = para_fft_pkg3d->nfft_proc;
 int ndata     = nfft_proc/2;

/*=======================================================================*/
/*  Unpack the data : Top half of k space only */

  for(i=1,m=1;i<=ndata;i++,m+=2){
    zfft[m] = rfft[i];
    zfft[(m+1)] = 0.0;
  }/*endfor*/

/*-----------------------------------------------------------------------*/
   }/*end routine*/ 
/*==========================================================================*/

/*==========================================================================*/
/*cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc*/
/*==========================================================================*/
/*  Dble upack the density */
/*==========================================================================*/

void dble_upack_rho(double *zfft,double *rfft1,double *rfft2,
                    PARA_FFT_PKG3D *para_fft_pkg3d)

/*=======================================================================*/
/*            Begin subprogram:                                          */
   {/*begin routine*/
/*=======================================================================*/
/*          Local variable declarations                                  */

 int i,m;
 int nfft_proc = para_fft_pkg3d->nfft_proc;
 int ndata     = nfft_proc/2;

/*=======================================================================*/
/*  Unpack the data : */

   for(i=1,m=1;i<=ndata;i++,m+=2){
    rfft1[i] = zfft[m];
    rfft2[i] = zfft[(m+1)];
   }/*endfor*/

/*-----------------------------------------------------------------------*/
   }/*end routine*/ 
/*==========================================================================*/


/*==========================================================================*/
/*cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc*/
/*==========================================================================*/
/*  Dble upack the density */
/*==========================================================================*/

void dble_upack_rho_scal(double *zfft,double *rfft1,double *rfft2,
                         PARA_FFT_PKG3D *para_fft_pkg3d,double scal)

/*=======================================================================*/
/*            Begin subprogram:                                          */
   {/*begin routine*/
/*=======================================================================*/
/*          Local variable declarations                                  */

 int i,m;
 int nfft_proc = para_fft_pkg3d->nfft_proc;
 int ndata     = nfft_proc/2;

/*=======================================================================*/
/*  Unpack the data : */

   for(i=1,m=1;i<=ndata;i++,m+=2){
    rfft1[i] = zfft[m]*scal;
    rfft2[i] = zfft[(m+1)]*scal;
   }/*endfor*/

/*-----------------------------------------------------------------------*/
   }/*end routine*/ 
/*==========================================================================*/


/*==========================================================================*/
/*cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc*/
/*==========================================================================*/
/*  Dble pack the density */
/*==========================================================================*/

void dble_pack_rho(double *zfft,double *rfft1,double *rfft2,
                   PARA_FFT_PKG3D *para_fft_pkg3d)

/*=======================================================================*/
/*            Begin subprogram:                                          */
   {/*begin routine*/
/*=======================================================================*/
/*          Local variable declarations                                  */

 int m,i;
 int nfft_proc = para_fft_pkg3d->nfft_proc;
 int ndata     = nfft_proc/2;

/*=======================================================================*/
/*  Unpack the data : Top half of k space only */

  for(i=1,m=1;i<=ndata;i++,m+=2){
    zfft[m] = rfft1[i];
    zfft[(m+1)] = rfft2[i];
  }/*endfor*/

/*-----------------------------------------------------------------------*/
   }/*end routine*/ 
/*==========================================================================*/




/*==========================================================================*/
/*cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc*/
/*==========================================================================*/
/*  Dble upack the coefs */
/*==========================================================================*/

void dble_upack_coef(double *c1re,double *c1im,double *c2re,double *c2im,
                     double *zfft,PARA_FFT_PKG3D *para_fft_pkg3d)

/*=======================================================================*/
/*            Begin subprogram:                                          */
   {/*begin routine*/
/*=======================================================================*/
/*          Local variable declarations                                  */

 int i,ncoef_min;
 double tempr ,tempi;
 double temprc,tempic;
 int ncoef_proc   = para_fft_pkg3d->ncoef_proc;
 int ncoef_use    = para_fft_pkg3d->ncoef_use;
 int *map_proc    = para_fft_pkg3d->map_proc;
 int *map_c_proc  = para_fft_pkg3d->map_c_proc;


/*=======================================================================*/
/*  Unpack the data :  */

  ncoef_min = MIN(ncoef_proc,ncoef_use);
  for(i=1;i<=ncoef_min;i++){
 
    tempr  = zfft[(map_proc[i])];
    tempi  = zfft[(map_proc[i]+1)];
    temprc = zfft[(map_c_proc[i])];
    tempic = zfft[(map_c_proc[i]+1)];

    c2im[i] = 0.5*(-tempr + temprc);
    c1re[i] = 0.5*( tempr + temprc);

    c1im[i] = 0.5*( tempi - tempic);
    c2re[i] = 0.5*( tempi + tempic);

  }/*endfor*/
  if(ncoef_proc>ncoef_use){
    i = ncoef_proc;
    c1im[i]=0.0;c2im[i]=0.0;
    c1re[i] = zfft[(map_proc[i])]; c2re[i] = zfft[(map_proc[i]+1)];
  }/*endif*/

/*-----------------------------------------------------------------------*/
   }/*end routine*/ 
/*==========================================================================*/





/*==========================================================================*/
/*cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc*/
/*==========================================================================*/
/*  Dble upack the coefs */
/*==========================================================================*/

void dble_upack_coef_sum(double *c1re,double *c1im,double *c2re,double *c2im,
                         double *zfft,PARA_FFT_PKG3D *para_fft_pkg3d)

/*=======================================================================*/
/*            Begin subprogram:                                          */
   {/*begin routine*/
/*=======================================================================*/
/*          Local variable declarations                                  */

 int i,ncoef_min;
 double tempr ,tempi;
 double temprc,tempic;
 int ncoef_proc   = para_fft_pkg3d->ncoef_proc;
 int ncoef_use    = para_fft_pkg3d->ncoef_use;
 int *map_proc    = para_fft_pkg3d->map_proc;
 int *map_c_proc  = para_fft_pkg3d->map_c_proc;

/*=======================================================================*/
/*  Unpack the data :  */

  ncoef_min = MIN(ncoef_proc,ncoef_use);
  for(i=1;i<=ncoef_min;i++){
 
    tempr  = zfft[(map_proc[i])];
    tempi  = zfft[(map_proc[i]+1)];
    temprc = zfft[(map_c_proc[i])];
    tempic = zfft[(map_c_proc[i]+1)];

    c2im[i] -= (2.0*(-tempr + temprc));
    c1re[i] -= (2.0*( tempr + temprc));

    c1im[i] -= (2.0*( tempi - tempic));
    c2re[i] -= (2.0*( tempi + tempic));

  }/*endfor*/
  if(ncoef_proc>ncoef_use){
    i = ncoef_proc;
    c1re[i] -= (2.0*zfft[(map_proc[i])]); 
    c2re[i] -= (2.0*zfft[(map_proc[i]+1)]);
  }/*endif*/

/*-----------------------------------------------------------------------*/
   }/*end routine*/ 
/*==========================================================================*/


/*==========================================================================*/
/*cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc*/
/*==========================================================================*/
/*  Sum the density */
/*==========================================================================*/

void sum_rho(double *zfft,double *rfft,PARA_FFT_PKG3D *para_fft_pkg3d)

/*=======================================================================*/
/*            Begin subprogram:                                          */
   {/*begin routine*/
/*=======================================================================*/
/*          Local variable declarations                                  */

 int m,i;
 int nfft_proc = para_fft_pkg3d->nfft_proc;
 int ndata     = nfft_proc/2;

/*=======================================================================*/
/*  Unpack the data : Top half of k space only */

  for(i=1,m=1;i<=ndata;i++,m+=2){
    rfft[i] += zfft[m]*zfft[m] + zfft[(m+1)]*zfft[(m+1)];
  }/*endfor*/

/*-----------------------------------------------------------------------*/
   }/*end routine*/ 
/*==========================================================================*/








/*==========================================================================*/
/*cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc*/
/*==========================================================================*/
/* zffts_gen : Fortran wrappers for 1D CP FFT                              */
/*==========================================================================*/

void fft_gen1d(double *zfft,int nfft,int incl,int num,int incn,
               int iopt,int *ier,double *work_1,int nwork_1,
               double *work_2,int nwork_2,int *ifax,int igeneric_opt)

/*==========================================================================*/
 {/*begin routine */
/*==========================================================================*/
/*   Local Variables */

 int ifound;
 int init        = 0;
 int nfft_for    = nfft;
 int incl_for_x  = incl;
 int incn_for_x  = incn;
 int incl_for_y  = incl;
 int incn_for_y  = incn;
 int num_for     = num;
 int nwork_1_for = nwork_1;
 int nwork_2_for = nwork_2;
 double scale    = 1.0;
 int iopt_for,i,istart;
 int ier_for;
 int isys;


/*==========================================================================*/
/* Machine Specific FFT */

 if(igeneric_opt==0){

  ifound = 0;

#ifdef IBM_ESSL
    ifound++;
    iopt_for = iopt;
    dcft(&init,&(zfft[1]),&incl_for_x,&incn_for_x,&(zfft[1]),
         &incl_for_y,&incn_for_y,&nfft_for,&num_for,&iopt_for,&scale,
         &(work_1[1]),&nwork_1_for,&(work_2[1]),&nwork_2_for);
#endif

#ifdef SGI_COMPLIB
    ifound++;
    iopt_for = -iopt;
    for(i=1;i<=num;i++){
      istart = 1 + (i-1)*2*incn;
      zfft1d_(&iopt_for,&nfft_for,&(zfft[istart]),&incl_for_x,&(work_2[1]));
    }/*endfor*/
#endif

#ifdef SUN_COMPLIB
   ifound++;
   if(iopt == 1){ /* forward  from g -> r */
    for(i=1;i<=num;i++){
      istart = 1 + (i-1)*2*incn;
      zfftf(nfft_for,&(zfft[istart]),&(work_2[1])); 
    }
   }else if(iopt == -1){ /* reverse from r -> g */
    for(i=1;i<=num;i++){
      istart = 1 + (i-1)*2*incn;
     zfftb(nfft_for,&(zfft[istart]),&(work_2[1])); 
    }
   }/*endif*/
#endif

#ifdef HP_VECLIB
    ifound++;
    iopt_for = iopt;
    zffts(&(zfft[1]),&nfft_for,&incl_for_x,&num_for,&incn_for_x,&iopt_for,
          &ier_for);
    *ier = ier_for;
    if(*ier!=0){
     printf("The HP is nice and dies if the radix conditions is wrong!!!\n");
     exit(1);
    }/*endif*/
#endif

#ifdef C90
   ifound++;
   iopt_for = -iopt;
   CFFT99(&(zfft[1]),&(work_1[1]),&(work_2[1]),&(ifax[1]),&incl_for_x,
          &incn_for_x,&nfft_for,&num_for,&iopt_for);
#endif


#ifdef T3E_SCILIB
  ifound++;
   isys  = 0;
   iopt_for = -iopt;
  for(i=1;i<=num;i++){
    istart = 1 + (i-1)*2*incn;
   CCFFT(&iopt_for,&nfft_for,&scale,&(zfft[istart]),&(zfft[istart]),
                 &(work_1[1]),&(work_2[1]),&isys);
    }/*endfor*/
#endif

   if(ifound!=1){
       printf("@@@@@@@@@@@@@@@@@@@@_error_@@@@@@@@@@@@@@@@@@@@\n");
       printf("Library FFT's not Implemented on this machine\n");
       printf("Use generic_fft_option in input file\n");
       printf("@@@@@@@@@@@@@@@@@@@@_error_@@@@@@@@@@@@@@@@@@@@\n");
       fflush(stdout);
       exit(1);
   }/*endif*/

/*==========================================================================*/
/* Generic FFT */

 }else{

   if(iopt == 1){

     for(i=1;i<=num;i++){
#ifdef T3E_SCILIB
       istart = 1 + (i-1)*2*incn;
       CFFTF_GENERIC(&nfft_for,&(zfft[istart]),&(work_2[1])); 
#else
       istart = 1 + (i-1)*2*incn;
       DCFFTF_GENERIC(&nfft_for,&(zfft[istart]),&(work_2[1])); 
#endif
     }/*endfor*/

   } else {

     for(i=1;i<=num;i++){
#ifdef T3E_SCILIB
       istart = 1 + (i-1)*2*incn;
       CFFTB_GENERIC(&nfft_for,&(zfft[istart]),&(work_2[1])); 
#else
       istart = 1 + (i-1)*2*incn;
       DCFFTB_GENERIC(&nfft_for,&(zfft[istart]),&(work_2[1])); 
#endif
     }/*endif*/

   }/* endif */

 }/* endif */

/*--------------------------------------------------------------------------*/
   }/*end routine*/
/*==========================================================================*/

/*==========================================================================*/
/*cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc*/
/*==========================================================================*/
/*  3D FFT from full r-space to full g-space  */
/*==========================================================================*/

void para_fft_gen3d_dvr(double *zfft, double *zfft_tmp,
                        PARA_FFT_PKG3D *para_fft_pkg3d,int idir)

/*=======================================================================*/
/*            Begin subprogram:                                          */
   {/*begin routine*/
/*=======================================================================*/
/*          Local variable declarations                                  */
#include "../typ_defs/typ_mask.h"

 int iopt = -1,incl=1;
 int ier  = 0;
 int incn;
 int i,j,k,m,ka,kb,kc,ioff,joff,koff,iii;
 int kap,kbp,kcp,kstr,kend,jb;
 int skc_fft_kb_now,ekc_fft_kb_now;
 int skb_fft_kb_now,ekb_fft_kb_now;
 int skb_fft_ka_now,ekb_fft_ka_now;
 int ndata,iproc;
 int kc_str,kc_end,kb_str,kb_end,ka_str,ka_end;
 double scale;
 int nfft;
 int kadd,madd;
 double isign;

/*-----------------------------------------------------------------------*/
/*          Local pointers                                               */

/* Communicator */
 MPI_Comm comm = para_fft_pkg3d->comm;
 int num_proc  = para_fft_pkg3d->num_proc;
 int myid      = para_fft_pkg3d->myid;
 int myidp1    = para_fft_pkg3d->myid+1;

/* General Info */
 int nktot        = para_fft_pkg3d->nktot;
 int igeneric_opt = para_fft_pkg3d->igeneric_opt;
 int scale_opt    = para_fft_pkg3d->scale_opt;
 int nwork1       = para_fft_pkg3d->nwork1;
 int nwork2       = para_fft_pkg3d->nwork2;

/* Map */
 int *map_proc_post = para_fft_pkg3d->map_proc_post;

/* FFT along kc  */
 int nkf3                = para_fft_pkg3d->nkf3;
 int *sendcounts_fft_kc  = para_fft_pkg3d->sendcounts_fft_kc;
 int *senddspls_fft_kc   = para_fft_pkg3d->senddspls_fft_kc;
 int *recvcounts_fft_kc  = para_fft_pkg3d->recvcounts_fft_kc;
 int *recvdspls_fft_kc   = para_fft_pkg3d->recvdspls_fft_kc;
 int nfft_kc_proc        = para_fft_pkg3d->nfft_kc_proc;
 int str_fft_kc_proc     = para_fft_pkg3d->str_fft_kc_proc;
 int end_fft_kc_proc     = para_fft_pkg3d->end_fft_kc_proc;
 int nka_fft_kc          = para_fft_pkg3d->nka_fft_kc;
 int *sum_fft_kc_proc    = para_fft_pkg3d->sum_fft_kc_proc;
 int *nkb_fft_kc         = para_fft_pkg3d->nkb_fft_kc;
 int *ka_fft_kc          = para_fft_pkg3d->ka_fft_kc;
 int *ka_fft_kc_red      = para_fft_pkg3d->ka_fft_kc_red;
 int *kb_fft_kc          = para_fft_pkg3d->kb_fft_kc;
 int ska_fft_kc_proc     = para_fft_pkg3d->ska_fft_kc_proc;
 int eka_fft_kc_proc     = para_fft_pkg3d->eka_fft_kc_proc;
 int skb_fft_kc_proc     = para_fft_pkg3d->skb_fft_kc_proc;
 int ekb_fft_kc_proc     = para_fft_pkg3d->ekb_fft_kc_proc;
 int *ifax_c_r           = para_fft_pkg3d->ifax_c_r;
 double *work_1c_r       = para_fft_pkg3d->work_1c_r;
 double *work_2c_r       = para_fft_pkg3d->work_2c_r;

/* FFT along kb  */
 int nkf2                 = para_fft_pkg3d->nkf2;
 int *sendcounts_fft_kb   = para_fft_pkg3d->sendcounts_fft_kb;
 int *senddspls_fft_kb    = para_fft_pkg3d->senddspls_fft_kb;
 int *recvcounts_fft_kb   = para_fft_pkg3d->recvcounts_fft_kb;
 int *recvdspls_fft_kb    = para_fft_pkg3d->recvdspls_fft_kb;
 int nfft_kb_proc         = para_fft_pkg3d->nfft_kb_proc;
 int str_fft_kb_proc      = para_fft_pkg3d->str_fft_kb_proc;
 int end_fft_kb_proc      = para_fft_pkg3d->end_fft_kb_proc;
 int ska_fft_kb_proc      = para_fft_pkg3d->ska_fft_kb_proc;
 int eka_fft_kb_proc      = para_fft_pkg3d->eka_fft_kb_proc;
 int skc_fft_kb_proc      = para_fft_pkg3d->skc_fft_kb_proc;
 int ekc_fft_kb_proc      = para_fft_pkg3d->ekc_fft_kb_proc;
 int *ska_fft_kb_proc_all = para_fft_pkg3d->ska_fft_kb_proc_all;
 int *eka_fft_kb_proc_all = para_fft_pkg3d->eka_fft_kb_proc_all;
 int *skc_fft_kb_proc_all = para_fft_pkg3d->skc_fft_kb_proc_all;
 int *ekc_fft_kb_proc_all = para_fft_pkg3d->ekc_fft_kb_proc_all;
 int *sum_fft_kb_proc     = para_fft_pkg3d->sum_fft_kb_proc;
 int *ifax_b_r            = para_fft_pkg3d->ifax_b_r;
 double *work_1b_r        = para_fft_pkg3d->work_1b_r;
 double *work_2b_r        = para_fft_pkg3d->work_2b_r;

/* FFT along ka  */
 int nkf1                 = para_fft_pkg3d->nkf1;
 int *sendcounts_fft_ka   = para_fft_pkg3d->sendcounts_fft_ka;
 int *senddspls_fft_ka    = para_fft_pkg3d->senddspls_fft_ka;
 int *recvcounts_fft_ka   = para_fft_pkg3d->recvcounts_fft_ka;
 int *recvdspls_fft_ka    = para_fft_pkg3d->recvdspls_fft_ka;
 int nfft_ka_proc         = para_fft_pkg3d->nfft_ka_proc;
 int str_fft_ka_proc      = para_fft_pkg3d->str_fft_ka_proc;
 int end_fft_ka_proc      = para_fft_pkg3d->end_fft_ka_proc;
 int skb_fft_ka_proc      = para_fft_pkg3d->skb_fft_ka_proc;
 int ekb_fft_ka_proc      = para_fft_pkg3d->ekb_fft_ka_proc;
 int skc_fft_ka_proc      = para_fft_pkg3d->skc_fft_ka_proc;
 int ekc_fft_ka_proc      = para_fft_pkg3d->ekc_fft_ka_proc;
 int *skb_fft_ka_proc_all = para_fft_pkg3d->skb_fft_ka_proc_all;
 int *ekb_fft_ka_proc_all = para_fft_pkg3d->ekb_fft_ka_proc_all;
 int *skc_fft_ka_proc_all = para_fft_pkg3d->skc_fft_ka_proc_all;
 int *ekc_fft_ka_proc_all = para_fft_pkg3d->ekc_fft_ka_proc_all;
 int *ifax_a_r            = para_fft_pkg3d->ifax_a_r;
 double *work_1a_r        = para_fft_pkg3d->work_1a_r;
 double *work_2a_r        = para_fft_pkg3d->work_2a_r;


#ifdef FFTW3
 /* FFTW3 */
 fftw_plan plan_ba = para_fft_pkg3d->plan_fftw_ba;
 fftw_plan plan_bb = para_fft_pkg3d->plan_fftw_bb;
 fftw_plan plan_bc = para_fft_pkg3d->plan_fftw_bc;
#endif


 int index1,index2;

  if(idir == 1){
    isign = -1.0;
  }else{
    isign =  1.0;
    iopt  = -iopt;
    scale_opt = 0;
#ifdef FFTW3
 /* FFTW3 */
 plan_ba = para_fft_pkg3d->plan_fftw_fa;
 plan_bb = para_fft_pkg3d->plan_fftw_fb;
 plan_bc = para_fft_pkg3d->plan_fftw_fc;
#endif
  }

/*==========================================================================*/
/* I) Perform Inverse FFT along ``a'' using zfft                               */

  nfft_ka_proc = nkf2*nkf3;
  nfft_kb_proc = nkf1*nkf3;
  nfft_kc_proc = nkf1*nkf2;

  incn = nkf1;

#ifdef FFT
#ifdef FFTW3
 if(igeneric_opt==0) {
   fftw_execute_dft(plan_ba,&(zfft[1]),&(zfft[1]));
 } else {
   fft_gen1d(zfft,nkf1,incl,nfft_ka_proc,incn,iopt,&ier,
            work_1a_r,nwork1,work_2a_r,nwork2,ifax_a_r,igeneric_opt);
 }

#else

  fft_gen1d(zfft,nkf1,incl,nfft_ka_proc,incn,iopt,&ier,
            work_1a_r,nwork1,work_2a_r,nwork2,ifax_a_r,igeneric_opt);
#endif
#endif


/*==========================================================================*/
/* II) Perform  Inverse FFT along ``b'' using zfft_tmp                          */

  i = 0;
  for(kc=1; kc <= nkf3; kc++){
  for(ka=1; ka <= nkf1; ka++){
  for(kb=1; kb <= nkf2; kb++){
    index1 = (kc-1)*nkf1*nkf2 + (kb-1)*nkf1 + (ka-1);
    zfft_tmp[2*i + 1] = zfft[2*index1 + 1];
    zfft_tmp[2*i + 2] = zfft[2*index1 + 2];
    i++;
  }}}

  incn = nkf2;

#ifdef FFT
#ifdef FFTW3
  /* backward ``b'' */
 if(igeneric_opt==0) {
   fftw_execute_dft(plan_bb,&(zfft_tmp[1]),&(zfft_tmp[1]));
 } else {
   fft_gen1d(zfft_tmp,nkf2,incl,nfft_kb_proc,incn,iopt,&ier,
            work_1b_r,nwork1,work_2b_r,nwork2,ifax_b_r,igeneric_opt);
 }
#else
  fft_gen1d(zfft_tmp,nkf2,incl,nfft_kb_proc,incn,iopt,&ier,
            work_1b_r,nwork1,work_2b_r,nwork2,ifax_b_r,igeneric_opt);
#endif
#endif

/*==========================================================================*/
/* III) Perform Inverse FFT along ``c'' using zfft                          */


  for(kc=1; kc <= nkf3; kc++){
  for(kb=1; kb <= nkf2; kb++){
  for(ka=1; ka <= nkf1; ka++){
    index1 = (ka-1)*nkf2*nkf3 + (kb-1)*nkf3 + (kc-1);  /* ia,ib,ic */
    index2 = (kc-1)*nkf2*nkf1 + (ka-1)*nkf2 + (kb-1);  /* ic,ia,ib */
    zfft[2*index1 + 1] = zfft_tmp[2*index2+1];
    zfft[2*index1 + 2] = zfft_tmp[2*index2+2];
  }}}

  incn = nkf3;

#ifdef FFT
#ifdef FFTW3
  /* backward ``c'' */
 if(igeneric_opt==0) {
   fftw_execute_dft(plan_bc,&(zfft[1]),&(zfft[1]));
 } else {
   fft_gen1d(zfft,nkf3,incl,nfft_kc_proc,incn,iopt,&ier,
            work_1c_r,nwork1,work_2c_r,nwork2,ifax_c_r,igeneric_opt);
 }
#else
  fft_gen1d(zfft,nkf3,incl,nfft_kc_proc,incn,iopt,&ier,
            work_1c_r,nwork1,work_2c_r,nwork2,ifax_c_r,igeneric_opt);
#endif
#endif

/*========================================================================*/
/* IV) Post-Processing */

  i = 0;
  for(ka=1; ka <= nkf1; ka++){
  for(kb=1; kb <= nkf2; kb++){
  for(kc=1; kc <= nkf3; kc++){
    index1 = (kc-1)*nkf1*nkf2 + (kb-1)*nkf1 + (ka-1);
    zfft_tmp[2*index1+1] =  zfft[2*i + 1];
    zfft_tmp[2*index1+2] =  isign*zfft[2*i + 2]; /* isign- DO NOT KNOW WHY YET */
    i++;
  }}}

  for(i=1; i <= 2*nkf1*nkf2*nkf3; i++){ zfft[i] = zfft_tmp[i]; }

/*======================================================================*/
/* V) scale zfft if necessary                                           */

  if(scale_opt == 1){
    ndata = 2*nfft_kc_proc*nkf3;
    nfft  = nkf1*nkf2*nkf3; scale = 1.0/((double) nfft);
    for(i=1;i<=ndata;i++){zfft[i] *= scale;}
  }/* endif */

/*-----------------------------------------------------------------------*/
}/*end routine*/
/*==========================================================================*/

/*==========================================================================*/
/*cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc*/
/*==========================================================================*/
/*  3D FFT from full g-space to full r-space  */
/*==========================================================================*/

void para_fft_gen3d_dvr_bck(double *zfft, double *zfft_tmp,
                            PARA_FFT_PKG3D *para_fft_pkg3d,int idir)

/*=======================================================================*/
/*            Begin subprogram:                                          */
   {/*begin routine*/
/*=======================================================================*/
/*          Local variable declarations                                  */
#include "../typ_defs/typ_mask.h"


 int iopt = -1,incl=1;
 int ier  = 0;
 int incn;
 int i,j,k,m,ka,kb,kc,ioff,joff,koff,iii;
 int kap,kbp,kcp,kstr,kend,jb;
 int skc_fft_kb_now,ekc_fft_kb_now;
 int skb_fft_kb_now,ekb_fft_kb_now;
 int skb_fft_ka_now,ekb_fft_ka_now;
 int ndata,iproc;
 int kc_str,kc_end,kb_str,kb_end,ka_str,ka_end;
 double scale;
 int nfft;
 int kadd,madd;
 double isign;

/*-----------------------------------------------------------------------*/
/*          Local pointers                                               */

/* Communicator */
 MPI_Comm comm = para_fft_pkg3d->comm;
 int num_proc  = para_fft_pkg3d->num_proc;
 int myid      = para_fft_pkg3d->myid;
 int myidp1    = para_fft_pkg3d->myid+1;

/* General Info */
 int nktot        = para_fft_pkg3d->nktot;
 int igeneric_opt = para_fft_pkg3d->igeneric_opt;
 int scale_opt    = para_fft_pkg3d->scale_opt;
 int nwork1       = para_fft_pkg3d->nwork1;
 int nwork2       = para_fft_pkg3d->nwork2;

/* Map */
 int *map_proc_post = para_fft_pkg3d->map_proc_post;

/* FFT along kc  */
 int nkf3                = para_fft_pkg3d->nkf3;
 int *sendcounts_fft_kc  = para_fft_pkg3d->sendcounts_fft_kc;
 int *senddspls_fft_kc   = para_fft_pkg3d->senddspls_fft_kc;
 int *recvcounts_fft_kc  = para_fft_pkg3d->recvcounts_fft_kc;
 int *recvdspls_fft_kc   = para_fft_pkg3d->recvdspls_fft_kc;
 int nfft_kc_proc        = para_fft_pkg3d->nfft_kc_proc;
 int nka_fft_kc          = para_fft_pkg3d->nka_fft_kc;
 int str_fft_kc_proc     = para_fft_pkg3d->str_fft_kc_proc;
 int end_fft_kc_proc     = para_fft_pkg3d->end_fft_kc_proc;
 int *sum_fft_kc_proc    = para_fft_pkg3d->sum_fft_kc_proc;
 int *ka_fft_kc          = para_fft_pkg3d->ka_fft_kc;
 int *kb_fft_kc          = para_fft_pkg3d->kb_fft_kc;
 int ska_fft_kc_proc     = para_fft_pkg3d->ska_fft_kc_proc;
 int eka_fft_kc_proc     = para_fft_pkg3d->eka_fft_kc_proc;
 int skb_fft_kc_proc     = para_fft_pkg3d->skb_fft_kc_proc;
 int ekb_fft_kc_proc     = para_fft_pkg3d->ekb_fft_kc_proc;
 int *nkb_fft_kc         = para_fft_pkg3d->nkb_fft_kc;
 int *ka_fft_kc_red      = para_fft_pkg3d->ka_fft_kc_red;
 int *ifax_c_f           = para_fft_pkg3d->ifax_c_f;
 double *work_1c_f       = para_fft_pkg3d->work_1c_f;
 double *work_2c_f       = para_fft_pkg3d->work_2c_f;


/* FFT along kb  */
 int nkf2                 = para_fft_pkg3d->nkf2;
 int *sendcounts_fft_kb   = para_fft_pkg3d->sendcounts_fft_kb;
 int *senddspls_fft_kb    = para_fft_pkg3d->senddspls_fft_kb;
 int *recvcounts_fft_kb   = para_fft_pkg3d->recvcounts_fft_kb;
 int *recvdspls_fft_kb    = para_fft_pkg3d->recvdspls_fft_kb;
 int nfft_kb_proc         = para_fft_pkg3d->nfft_kb_proc;
 int str_fft_kb_proc      = para_fft_pkg3d->str_fft_kb_proc;
 int end_fft_kb_proc      = para_fft_pkg3d->end_fft_kb_proc;
 int ska_fft_kb_proc      = para_fft_pkg3d->ska_fft_kb_proc;
 int eka_fft_kb_proc      = para_fft_pkg3d->eka_fft_kb_proc;
 int skc_fft_kb_proc      = para_fft_pkg3d->skc_fft_kb_proc;
 int ekc_fft_kb_proc      = para_fft_pkg3d->ekc_fft_kb_proc;
 int *ska_fft_kb_proc_all = para_fft_pkg3d->ska_fft_kb_proc_all;
 int *eka_fft_kb_proc_all = para_fft_pkg3d->eka_fft_kb_proc_all;
 int *skc_fft_kb_proc_all = para_fft_pkg3d->skc_fft_kb_proc_all;
 int *ekc_fft_kb_proc_all = para_fft_pkg3d->ekc_fft_kb_proc_all;
 int *sum_fft_kb_proc     = para_fft_pkg3d->sum_fft_kb_proc;
 int *ifax_b_f            = para_fft_pkg3d->ifax_b_f;
 double *work_1b_f       = para_fft_pkg3d->work_1b_f;
 double *work_2b_f       = para_fft_pkg3d->work_2b_f;

/* FFT along ka  */
 int nkf1                 = para_fft_pkg3d->nkf1;
 int *sendcounts_fft_ka   = para_fft_pkg3d->sendcounts_fft_ka;
 int *senddspls_fft_ka    = para_fft_pkg3d->senddspls_fft_ka;
 int *recvcounts_fft_ka   = para_fft_pkg3d->recvcounts_fft_ka;
 int *recvdspls_fft_ka    = para_fft_pkg3d->recvdspls_fft_ka;
 int nfft_ka_proc         = para_fft_pkg3d->nfft_ka_proc;
 int str_fft_ka_proc      = para_fft_pkg3d->str_fft_ka_proc;
 int end_fft_ka_proc      = para_fft_pkg3d->end_fft_ka_proc;
 int skb_fft_ka_proc      = para_fft_pkg3d->skb_fft_ka_proc;
 int ekb_fft_ka_proc      = para_fft_pkg3d->ekb_fft_ka_proc;
 int skc_fft_ka_proc      = para_fft_pkg3d->skc_fft_ka_proc;
 int ekc_fft_ka_proc      = para_fft_pkg3d->ekc_fft_ka_proc;
 int *skb_fft_ka_proc_all = para_fft_pkg3d->skb_fft_ka_proc_all;
 int *ekb_fft_ka_proc_all = para_fft_pkg3d->ekb_fft_ka_proc_all;
 int *skc_fft_ka_proc_all = para_fft_pkg3d->skc_fft_ka_proc_all;
 int *ekc_fft_ka_proc_all = para_fft_pkg3d->ekc_fft_ka_proc_all;
 int *ifax_a_f            = para_fft_pkg3d->ifax_a_f;
 double *work_1a_f       = para_fft_pkg3d->work_1a_f;
 double *work_2a_f       = para_fft_pkg3d->work_2a_f;

#ifdef FFTW3
 /* FFTW3 */
 fftw_plan plan_ba = para_fft_pkg3d->plan_fftw_ba;
 fftw_plan plan_bb = para_fft_pkg3d->plan_fftw_bb;
 fftw_plan plan_bc = para_fft_pkg3d->plan_fftw_bc;
#endif

 int index1,index2;

 if(idir == 1){
   isign = -1.0;
 }else{
   isign = -1.0;
   iopt  = -iopt;
   scale_opt = 0;
#ifdef FFTW3
 /* FFTW3 */
 plan_ba = para_fft_pkg3d->plan_fftw_fa;
 plan_bb = para_fft_pkg3d->plan_fftw_fb;
 plan_bc = para_fft_pkg3d->plan_fftw_fc;
#endif
 }

/*==========================================================================*/
/* I) Perform FFT along ``c'' using zfft                               */

  nfft_ka_proc = nkf2*nkf3;
  nfft_kb_proc = nkf1*nkf3;
  nfft_kc_proc = nkf1*nkf2;

  incn = nkf3;

  /* Make c contiguous in memory */
   for(i=1; i <= nkf3*nkf2*nkf1*2; i++){ zfft_tmp[i] = zfft[i];}

    i = 0;
   for(kc=1; kc <= nkf3; kc++){
   for(ka=1; ka <= nkf1; ka++){
   for(kb=1; kb <= nkf2; kb++){
     index1 = (kc-1)*nkf2*nkf1 + (kb-1)*nkf1 + (ka-1);
     index2 = (ka-1)*nkf2*nkf3 + (kb-1)*nkf3 + (kc-1);
     zfft[2*index2 + 1] = zfft_tmp[2*index1 + 1];
     zfft[2*index2 + 2] = zfft_tmp[2*index1 + 2];
     i++;
   }}}

#ifdef FFT
#ifdef FFTW3
 /* forward ``c'' */
 if(igeneric_opt==0) {
   fftw_execute_dft(plan_bc,&(zfft[1]),&(zfft[1]));
 } else {
   fft_gen1d(zfft,nkf3,incl,nfft_kc_proc,incn,iopt,&ier,
            work_1c_f,nwork1,work_2c_f,nwork2,ifax_c_f,igeneric_opt);
 }

#else
  fft_gen1d(zfft,nkf3,incl,nfft_kc_proc,incn,iopt,&ier,
            work_1c_f,nwork1,work_2c_f,nwork2,ifax_c_f,igeneric_opt);
#endif
#endif

/*==========================================================================*/
/* II) Perform  Inverse FFT along ``b'' using zfft_tmp                          */

  /* Make b contiguous in memory */
    i = 0;
   for(kc=1; kc <= nkf3; kc++){
   for(ka=1; ka <= nkf1; ka++){
   for(kb=1; kb <= nkf2; kb++){
     index1 = (ka-1)*nkf2*nkf3 + (kb-1)*nkf3 + (kc-1);
     index2 = (ka-1)*nkf3*nkf2 + (kc-1)*nkf2 + (kb-1);
     zfft_tmp[2*index2 + 1] = zfft[2*index1 + 1];
     zfft_tmp[2*index2 + 2] = zfft[2*index1 + 2];
     i++;
   }}}

  incn = nkf2;

#ifdef FFT
#ifdef FFTW3
  /* forward ``b'' */
 if(igeneric_opt==0) {
 fftw_execute_dft(plan_bb,&(zfft_tmp[1]),&(zfft_tmp[1]));
 } else {
   fft_gen1d(zfft_tmp,nkf2,incl,nfft_kb_proc,incn,iopt,&ier,
            work_1b_f,nwork1,work_2b_f,nwork2,ifax_b_f,igeneric_opt);
 }

#else
  fft_gen1d(zfft_tmp,nkf2,incl,nfft_kb_proc,incn,iopt,&ier,
            work_1b_f,nwork1,work_2b_f,nwork2,ifax_b_f,igeneric_opt);
#endif
#endif
/*==========================================================================*/
/* III) Perform Inverse FFT along ``a'' using zfft                               */

 /* a contiguous in memory */

  for(ka=1; ka <= nkf1; ka++){
  for(kb=1; kb <= nkf2; kb++){
  for(kc=1; kc <= nkf3; kc++){
    index1 = (ka-1)*nkf3*nkf2 + (kc-1)*nkf2 + (kb-1);
    index2 = (kc-1)*nkf1*nkf2 + (kb-1)*nkf1 + (ka-1);
    zfft[2*index2+1] = zfft_tmp[2*index1+1];
    zfft[2*index2+2] = zfft_tmp[2*index1+2];
  }}}

  incn = nkf1;

#ifdef FFT
#ifdef FFTW3
  /* forward ``a'' */
 if(igeneric_opt==0) {
 fftw_execute_dft(plan_ba,&(zfft[1]),&(zfft[1]));
 } else {
    fft_gen1d(zfft,nkf1,incl,nfft_ka_proc,incn,iopt,&ier,
            work_1a_f,nwork1,work_2a_f,nwork2,ifax_a_f,igeneric_opt);
 }
#else
  fft_gen1d(zfft,nkf1,incl,nfft_ka_proc,incn,iopt,&ier,
            work_1a_f,nwork1,work_2a_f,nwork2,ifax_a_f,igeneric_opt);
#endif
#endif

/*=======================================================================*/
/* IV) scale zfft if necessary                                           */

  if(scale_opt == 1){
    ndata = 2*nfft_kc_proc*nkf3;
    nfft  = nkf1*nkf2*nkf3; scale = 1.0/((double) nfft);
    for(i=1;i<=ndata;i++){zfft[i] *= scale;}
  }/* endif */

/*-----------------------------------------------------------------------*/
}/*end routine*/
/*==========================================================================*/



