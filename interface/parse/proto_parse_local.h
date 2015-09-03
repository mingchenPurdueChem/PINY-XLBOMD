void control_zero_mem(CLASS *,BONDED *,GENERAL_DATA *,CP *,CLASS_PARSE *,
                      NULL_INTER_PARSE *);

void mpi_malloc(CLASS *,BONDED *,CP *,NULL_INTER_PARSE *,int);

void zero_sys(CLASS *,GENERAL_DATA *);

void zero_bnd(BONDED *);

void zero_cp(CP *);

void zero_par(CLASS_PARSE *,NULL_INTER_PARSE *);

void control_init_cp_orthog(GENERAL_DATA *,CP *,CP_PARSE *,int ,int ,int);


void control_init_class_vsmpl(CLASS *,CLASS_PARSE *,BONDED *,GENERAL_DATA *);
                   

void control_init_coef_vsmpl(GENERAL_DATA *,CP *,CP_PARSE *,int);

void make_cp_atom_list(ATOMMAPS *,int *,int *,int );

void calculate_cp_nfree(CP *);

/*--------------------------------------------------------*/
/* DVR related */
void calculate_cp_nfree_dvr(CP *);
void move_to_cell_center(CLASS *);
void gen_Ts_dvr(DVR_MATRIX *,CPCOEFFS_INFO *, CELL *,int ,int );
void gen_dgrid_dvr(DVR_MATRIX *, CPCOEFFS_INFO *, CELL *,int ,int );
void gen_dvr_fbr(DVR_MATRIX *, CPCOEFFS_INFO *, CELL *, CPSCR_WAVE *,
                 int ,int , double );



