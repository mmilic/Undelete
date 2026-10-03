#
# Generated Makefile - do not edit!
#
# Edit the Makefile in the project folder instead (../Makefile). Each target
# has a -pre and a -post target defined where you can add customized code.
#
# This makefile implements configuration specific macros and targets.


# Environment
MKDIR=mkdir
CP=cp
GREP=grep
NM=nm
CCADMIN=CCadmin
RANLIB=ranlib
CC=gcc
CCC=g++
CXX=g++
FC=gfortran
AS=as

# Macros
CND_PLATFORM=MinGW-Windows
CND_DLIB_EXT=dll
CND_CONF=Debug
CND_DISTDIR=dist
CND_BUILDDIR=build

# Include project Makefile
include Makefile

# Object Directory
OBJECTDIR=${CND_BUILDDIR}/${CND_CONF}/${CND_PLATFORM}

# Object Files
OBJECTFILES= \
	${OBJECTDIR}/CustomButton.o \
	${OBJECTDIR}/FAT/addpart.o \
	${OBJECTDIR}/FAT/analyse.o \
	${OBJECTDIR}/FAT/askloc.o \
	${OBJECTDIR}/FAT/autoset.o \
	${OBJECTDIR}/FAT/bfs.o \
	${OBJECTDIR}/FAT/bsd.o \
	${OBJECTDIR}/FAT/btrfs.o \
	${OBJECTDIR}/FAT/chgtype.o \
	${OBJECTDIR}/FAT/chgtypen.o \
	${OBJECTDIR}/FAT/common.o \
	${OBJECTDIR}/FAT/cramfs.o \
	${OBJECTDIR}/FAT/crc.o \
	${OBJECTDIR}/FAT/dfxml.o \
	${OBJECTDIR}/FAT/dir.o \
	${OBJECTDIR}/FAT/ewf.o \
	${OBJECTDIR}/FAT/exfat.o \
	${OBJECTDIR}/FAT/exfatp.o \
	${OBJECTDIR}/FAT/ext2.o \
	${OBJECTDIR}/FAT/ext2_dir.o \
	${OBJECTDIR}/FAT/ext2grp.o \
	${OBJECTDIR}/FAT/ext2p.o \
	${OBJECTDIR}/FAT/fat.o \
	${OBJECTDIR}/FAT/fat_cluster.o \
	${OBJECTDIR}/FAT/fat_dir.o \
	${OBJECTDIR}/FAT/fat_unformat.o \
	${OBJECTDIR}/FAT/fatp.o \
	${OBJECTDIR}/FAT/fatx.o \
	${OBJECTDIR}/FAT/file_1cd.o \
	${OBJECTDIR}/FAT/file_7z.o \
	${OBJECTDIR}/FAT/file_DB.o \
	${OBJECTDIR}/FAT/file_a.o \
	${OBJECTDIR}/FAT/file_ab.o \
	${OBJECTDIR}/FAT/file_abcdp.o \
	${OBJECTDIR}/FAT/file_abr.o \
	${OBJECTDIR}/FAT/file_acb.o \
	${OBJECTDIR}/FAT/file_ace.o \
	${OBJECTDIR}/FAT/file_ado.o \
	${OBJECTDIR}/FAT/file_ahn.o \
	${OBJECTDIR}/FAT/file_aif.o \
	${OBJECTDIR}/FAT/file_all.o \
	${OBJECTDIR}/FAT/file_als.o \
	${OBJECTDIR}/FAT/file_amd.o \
	${OBJECTDIR}/FAT/file_amr.o \
	${OBJECTDIR}/FAT/file_apa.o \
	${OBJECTDIR}/FAT/file_ape.o \
	${OBJECTDIR}/FAT/file_apple.o \
	${OBJECTDIR}/FAT/file_arj.o \
	${OBJECTDIR}/FAT/file_asf.o \
	${OBJECTDIR}/FAT/file_asl.o \
	${OBJECTDIR}/FAT/file_asm.o \
	${OBJECTDIR}/FAT/file_atd.o \
	${OBJECTDIR}/FAT/file_au.o \
	${OBJECTDIR}/FAT/file_axx.o \
	${OBJECTDIR}/FAT/file_bac.o \
	${OBJECTDIR}/FAT/file_bim.o \
	${OBJECTDIR}/FAT/file_binvox.o \
	${OBJECTDIR}/FAT/file_bkf.o \
	${OBJECTDIR}/FAT/file_bld.o \
	${OBJECTDIR}/FAT/file_bmp.o \
	${OBJECTDIR}/FAT/file_bz2.o \
	${OBJECTDIR}/FAT/file_cab.o \
	${OBJECTDIR}/FAT/file_caf.o \
	${OBJECTDIR}/FAT/file_cam.o \
	${OBJECTDIR}/FAT/file_catdrawing.o \
	${OBJECTDIR}/FAT/file_cdt.o \
	${OBJECTDIR}/FAT/file_chm.o \
	${OBJECTDIR}/FAT/file_class.o \
	${OBJECTDIR}/FAT/file_cm.o \
	${OBJECTDIR}/FAT/file_compress.o \
	${OBJECTDIR}/FAT/file_cow.o \
	${OBJECTDIR}/FAT/file_crw.o \
	${OBJECTDIR}/FAT/file_csh.o \
	${OBJECTDIR}/FAT/file_ctg.o \
	${OBJECTDIR}/FAT/file_cwk.o \
	${OBJECTDIR}/FAT/file_d2s.o \
	${OBJECTDIR}/FAT/file_dar.o \
	${OBJECTDIR}/FAT/file_dat.o \
	${OBJECTDIR}/FAT/file_dbf.o \
	${OBJECTDIR}/FAT/file_dbn.o \
	${OBJECTDIR}/FAT/file_ddf.o \
	${OBJECTDIR}/FAT/file_dex.o \
	${OBJECTDIR}/FAT/file_dim.o \
	${OBJECTDIR}/FAT/file_dir.o \
	${OBJECTDIR}/FAT/file_djv.o \
	${OBJECTDIR}/FAT/file_dmp.o \
	${OBJECTDIR}/FAT/file_doc.o \
	${OBJECTDIR}/FAT/file_dpx.o \
	${OBJECTDIR}/FAT/file_drw.o \
	${OBJECTDIR}/FAT/file_ds2.o \
	${OBJECTDIR}/FAT/file_dsc.o \
	${OBJECTDIR}/FAT/file_dss.o \
	${OBJECTDIR}/FAT/file_dta.o \
	${OBJECTDIR}/FAT/file_dump.o \
	${OBJECTDIR}/FAT/file_dv.o \
	${OBJECTDIR}/FAT/file_dwg.o \
	${OBJECTDIR}/FAT/file_dxf.o \
	${OBJECTDIR}/FAT/file_e01.o \
	${OBJECTDIR}/FAT/file_ecryptfs.o \
	${OBJECTDIR}/FAT/file_edb.o \
	${OBJECTDIR}/FAT/file_elf.o \
	${OBJECTDIR}/FAT/file_emf.o \
	${OBJECTDIR}/FAT/file_evt.o \
	${OBJECTDIR}/FAT/file_exe.o \
	${OBJECTDIR}/FAT/file_exs.o \
	${OBJECTDIR}/FAT/file_ext.o \
	${OBJECTDIR}/FAT/file_ext2.o \
	${OBJECTDIR}/FAT/file_fat.o \
	${OBJECTDIR}/FAT/file_fbf.o \
	${OBJECTDIR}/FAT/file_fbk.o \
	${OBJECTDIR}/FAT/file_fcp.o \
	${OBJECTDIR}/FAT/file_fcs.o \
	${OBJECTDIR}/FAT/file_fdb.o \
	${OBJECTDIR}/FAT/file_fh10.o \
	${OBJECTDIR}/FAT/file_fh5.o \
	${OBJECTDIR}/FAT/file_filevault.o \
	${OBJECTDIR}/FAT/file_fits.o \
	${OBJECTDIR}/FAT/file_flac.o \
	${OBJECTDIR}/FAT/file_flp.o \
	${OBJECTDIR}/FAT/file_flv.o \
	${OBJECTDIR}/FAT/file_fob.o \
	${OBJECTDIR}/FAT/file_found.o \
	${OBJECTDIR}/FAT/file_fp5.o \
	${OBJECTDIR}/FAT/file_fp7.o \
	${OBJECTDIR}/FAT/file_freeway.o \
	${OBJECTDIR}/FAT/file_frm.o \
	${OBJECTDIR}/FAT/file_fs.o \
	${OBJECTDIR}/FAT/file_fwd.o \
	${OBJECTDIR}/FAT/file_gam.o \
	${OBJECTDIR}/FAT/file_gct.o \
	${OBJECTDIR}/FAT/file_gho.o \
	${OBJECTDIR}/FAT/file_gif.o \
	${OBJECTDIR}/FAT/file_gm6.o \
	${OBJECTDIR}/FAT/file_gp5.o \
	${OBJECTDIR}/FAT/file_gpg.o \
	${OBJECTDIR}/FAT/file_gz.o \
	${OBJECTDIR}/FAT/file_hdf.o \
	${OBJECTDIR}/FAT/file_hds.o \
	${OBJECTDIR}/FAT/file_hfsp.o \
	${OBJECTDIR}/FAT/file_hr9.o \
	${OBJECTDIR}/FAT/file_http.o \
	${OBJECTDIR}/FAT/file_icc.o \
	${OBJECTDIR}/FAT/file_ico.o \
	${OBJECTDIR}/FAT/file_ifo.o \
	${OBJECTDIR}/FAT/file_imb.o \
	${OBJECTDIR}/FAT/file_indd.o \
	${OBJECTDIR}/FAT/file_info.o \
	${OBJECTDIR}/FAT/file_iso.o \
	${OBJECTDIR}/FAT/file_it.o \
	${OBJECTDIR}/FAT/file_itu.o \
	${OBJECTDIR}/FAT/file_jpg.o \
	${OBJECTDIR}/FAT/file_kdb.o \
	${OBJECTDIR}/FAT/file_kdbx.o \
	${OBJECTDIR}/FAT/file_ldf.o \
	${OBJECTDIR}/FAT/file_list.o \
	${OBJECTDIR}/FAT/file_lit.o \
	${OBJECTDIR}/FAT/file_lnk.o \
	${OBJECTDIR}/FAT/file_logic.o \
	${OBJECTDIR}/FAT/file_lso.o \
	${OBJECTDIR}/FAT/file_lxo.o \
	${OBJECTDIR}/FAT/file_lzo.o \
	${OBJECTDIR}/FAT/file_m2ts.o \
	${OBJECTDIR}/FAT/file_mat.o \
	${OBJECTDIR}/FAT/file_max.o \
	${OBJECTDIR}/FAT/file_mb.o \
	${OBJECTDIR}/FAT/file_mcd.o \
	${OBJECTDIR}/FAT/file_mdb.o \
	${OBJECTDIR}/FAT/file_mdf.o \
	${OBJECTDIR}/FAT/file_mfa.o \
	${OBJECTDIR}/FAT/file_mfg.o \
	${OBJECTDIR}/FAT/file_mft.o \
	${OBJECTDIR}/FAT/file_mid.o \
	${OBJECTDIR}/FAT/file_mig.o \
	${OBJECTDIR}/FAT/file_mk5.o \
	${OBJECTDIR}/FAT/file_mkv.o \
	${OBJECTDIR}/FAT/file_mobi.o \
	${OBJECTDIR}/FAT/file_mov.o \
	${OBJECTDIR}/FAT/file_mp3.o \
	${OBJECTDIR}/FAT/file_mpg.o \
	${OBJECTDIR}/FAT/file_mrw.o \
	${OBJECTDIR}/FAT/file_mus.o \
	${OBJECTDIR}/FAT/file_mxf.o \
	${OBJECTDIR}/FAT/file_myo.o \
	${OBJECTDIR}/FAT/file_mysql.o \
	${OBJECTDIR}/FAT/file_nds.o \
	${OBJECTDIR}/FAT/file_njx.o \
	${OBJECTDIR}/FAT/file_nk2.o \
	${OBJECTDIR}/FAT/file_nsf.o \
	${OBJECTDIR}/FAT/file_oci.o \
	${OBJECTDIR}/FAT/file_ogg.o \
	${OBJECTDIR}/FAT/file_one.o \
	${OBJECTDIR}/FAT/file_orf.o \
	${OBJECTDIR}/FAT/file_paf.o \
	${OBJECTDIR}/FAT/file_pap.o \
	${OBJECTDIR}/FAT/file_par2.o \
	${OBJECTDIR}/FAT/file_pcap.o \
	${OBJECTDIR}/FAT/file_pct.o \
	${OBJECTDIR}/FAT/file_pcx.o \
	${OBJECTDIR}/FAT/file_pdf.o \
	${OBJECTDIR}/FAT/file_pds.o \
	${OBJECTDIR}/FAT/file_pfx.o \
	${OBJECTDIR}/FAT/file_plt.o \
	${OBJECTDIR}/FAT/file_png.o \
	${OBJECTDIR}/FAT/file_pnm.o \
	${OBJECTDIR}/FAT/file_prc.o \
	${OBJECTDIR}/FAT/file_prt.o \
	${OBJECTDIR}/FAT/file_ps.o \
	${OBJECTDIR}/FAT/file_psd.o \
	${OBJECTDIR}/FAT/file_psf.o \
	${OBJECTDIR}/FAT/file_psp.o \
	${OBJECTDIR}/FAT/file_pst.o \
	${OBJECTDIR}/FAT/file_ptb.o \
	${OBJECTDIR}/FAT/file_ptf.o \
	${OBJECTDIR}/FAT/file_pyc.o \
	${OBJECTDIR}/FAT/file_pzf.o \
	${OBJECTDIR}/FAT/file_pzh.o \
	${OBJECTDIR}/FAT/file_qbb.o \
	${OBJECTDIR}/FAT/file_qdf.o \
	${OBJECTDIR}/FAT/file_qkt.o \
	${OBJECTDIR}/FAT/file_qxd.o \
	${OBJECTDIR}/FAT/file_r3d.o \
	${OBJECTDIR}/FAT/file_ra.o \
	${OBJECTDIR}/FAT/file_raf.o \
	${OBJECTDIR}/FAT/file_rar.o \
	${OBJECTDIR}/FAT/file_raw.o \
	${OBJECTDIR}/FAT/file_rdc.o \
	${OBJECTDIR}/FAT/file_reg.o \
	${OBJECTDIR}/FAT/file_res.o \
	${OBJECTDIR}/FAT/file_rfp.o \
	${OBJECTDIR}/FAT/file_riff.o \
	${OBJECTDIR}/FAT/file_rm.o \
	${OBJECTDIR}/FAT/file_rns.o \
	${OBJECTDIR}/FAT/file_rpm.o \
	${OBJECTDIR}/FAT/file_rw2.o \
	${OBJECTDIR}/FAT/file_rx2.o \
	${OBJECTDIR}/FAT/file_save.o \
	${OBJECTDIR}/FAT/file_ses.o \
	${OBJECTDIR}/FAT/file_sib.o \
	${OBJECTDIR}/FAT/file_sig.o \
	${OBJECTDIR}/FAT/file_sit.o \
	${OBJECTDIR}/FAT/file_skd.o \
	${OBJECTDIR}/FAT/file_skp.o \
	${OBJECTDIR}/FAT/file_sp3.o \
	${OBJECTDIR}/FAT/file_spe.o \
	${OBJECTDIR}/FAT/file_spf.o \
	${OBJECTDIR}/FAT/file_spss.o \
	${OBJECTDIR}/FAT/file_sql.o \
	${OBJECTDIR}/FAT/file_sqm.o \
	${OBJECTDIR}/FAT/file_stl.o \
	${OBJECTDIR}/FAT/file_stu.o \
	${OBJECTDIR}/FAT/file_swf.o \
	${OBJECTDIR}/FAT/file_tar.o \
	${OBJECTDIR}/FAT/file_tax.o \
	${OBJECTDIR}/FAT/file_tib.o \
	${OBJECTDIR}/FAT/file_tiff.o \
	${OBJECTDIR}/FAT/file_tivo.o \
	${OBJECTDIR}/FAT/file_torrent.o \
	${OBJECTDIR}/FAT/file_tph.o \
	${OBJECTDIR}/FAT/file_tpl.o \
	${OBJECTDIR}/FAT/file_ttf.o \
	${OBJECTDIR}/FAT/file_txt.o \
	${OBJECTDIR}/FAT/file_tz.o \
	${OBJECTDIR}/FAT/file_v2i.o \
	${OBJECTDIR}/FAT/file_vault.o \
	${OBJECTDIR}/FAT/file_vdi.o \
	${OBJECTDIR}/FAT/file_veg.o \
	${OBJECTDIR}/FAT/file_vfb.o \
	${OBJECTDIR}/FAT/file_vmdk.o \
	${OBJECTDIR}/FAT/file_vmg.o \
	${OBJECTDIR}/FAT/file_wdp.o \
	${OBJECTDIR}/FAT/file_win.o \
	${OBJECTDIR}/FAT/file_wks.o \
	${OBJECTDIR}/FAT/file_wmf.o \
	${OBJECTDIR}/FAT/file_wnk.o \
	${OBJECTDIR}/FAT/file_wpb.o \
	${OBJECTDIR}/FAT/file_wpd.o \
	${OBJECTDIR}/FAT/file_wtv.o \
	${OBJECTDIR}/FAT/file_wv.o \
	${OBJECTDIR}/FAT/file_x3f.o \
	${OBJECTDIR}/FAT/file_xcf.o \
	${OBJECTDIR}/FAT/file_xfi.o \
	${OBJECTDIR}/FAT/file_xm.o \
	${OBJECTDIR}/FAT/file_xpt.o \
	${OBJECTDIR}/FAT/file_xsv.o \
	${OBJECTDIR}/FAT/file_xv.o \
	${OBJECTDIR}/FAT/file_xz.o \
	${OBJECTDIR}/FAT/file_zip.o \
	${OBJECTDIR}/FAT/filegen.o \
	${OBJECTDIR}/FAT/fnctdsk.o \
	${OBJECTDIR}/FAT/geometry.o \
	${OBJECTDIR}/FAT/gfs2.o \
	${OBJECTDIR}/FAT/hdaccess.o \
	${OBJECTDIR}/FAT/hdcache.o \
	${OBJECTDIR}/FAT/hdwin32.o \
	${OBJECTDIR}/FAT/hfs.o \
	${OBJECTDIR}/FAT/hfsp.o \
	${OBJECTDIR}/FAT/hidden.o \
	${OBJECTDIR}/FAT/hiddenn.o \
	${OBJECTDIR}/FAT/hpa_dco.o \
	${OBJECTDIR}/FAT/hpfs.o \
	${OBJECTDIR}/FAT/intrf.o \
	${OBJECTDIR}/FAT/intrfn.o \
	${OBJECTDIR}/FAT/iso.o \
	${OBJECTDIR}/FAT/jfs.o \
	${OBJECTDIR}/FAT/list.o \
	${OBJECTDIR}/FAT/list_sort.o \
	${OBJECTDIR}/FAT/log.o \
	${OBJECTDIR}/FAT/log_part.o \
	${OBJECTDIR}/FAT/luks.o \
	${OBJECTDIR}/FAT/lvm.o \
	${OBJECTDIR}/FAT/md.o \
	${OBJECTDIR}/FAT/misc.o \
	${OBJECTDIR}/FAT/netware.o \
	${OBJECTDIR}/FAT/nodisk.o \
	${OBJECTDIR}/FAT/ntfs.o \
	${OBJECTDIR}/FAT/ntfs_dir.o \
	${OBJECTDIR}/FAT/ntfs_utl.o \
	${OBJECTDIR}/FAT/ntfsp.o \
	${OBJECTDIR}/FAT/partauto.o \
	${OBJECTDIR}/FAT/partgpt.o \
	${OBJECTDIR}/FAT/partgptn.o \
	${OBJECTDIR}/FAT/partgptro.o \
	${OBJECTDIR}/FAT/parthumax.o \
	${OBJECTDIR}/FAT/parti386.o \
	${OBJECTDIR}/FAT/parti386n.o \
	${OBJECTDIR}/FAT/partmac.o \
	${OBJECTDIR}/FAT/partmacn.o \
	${OBJECTDIR}/FAT/partnone.o \
	${OBJECTDIR}/FAT/partsun.o \
	${OBJECTDIR}/FAT/partsunn.o \
	${OBJECTDIR}/FAT/partxbox.o \
	${OBJECTDIR}/FAT/partxboxn.o \
	${OBJECTDIR}/FAT/pblocksize.o \
	${OBJECTDIR}/FAT/pdisksel.o \
	${OBJECTDIR}/FAT/pfree_whole.o \
	${OBJECTDIR}/FAT/phbf.o \
	${OBJECTDIR}/FAT/phbs.o \
	${OBJECTDIR}/FAT/phcfg.o \
	${OBJECTDIR}/FAT/phmain.o \
	${OBJECTDIR}/FAT/phnc.o \
	${OBJECTDIR}/FAT/photorec.o \
	${OBJECTDIR}/FAT/phrecn.o \
	${OBJECTDIR}/FAT/ppartsel.o \
	${OBJECTDIR}/FAT/rfs.o \
	${OBJECTDIR}/FAT/savehdr.o \
	${OBJECTDIR}/FAT/sessionp.o \
	${OBJECTDIR}/FAT/setdate.o \
	${OBJECTDIR}/FAT/sudo.o \
	${OBJECTDIR}/FAT/sun.o \
	${OBJECTDIR}/FAT/swap.o \
	${OBJECTDIR}/FAT/sysv.o \
	${OBJECTDIR}/FAT/ufs.o \
	${OBJECTDIR}/FAT/unicode.o \
	${OBJECTDIR}/FAT/vmfs.o \
	${OBJECTDIR}/FAT/wbfs.o \
	${OBJECTDIR}/FAT/win32.o \
	${OBJECTDIR}/FAT/xfs.o \
	${OBJECTDIR}/FAT/zfs.o \
	${OBJECTDIR}/IsValidFileName.o \
	${OBJECTDIR}/MFTRecord.o \
	${OBJECTDIR}/MyThreadClass.o \
	${OBJECTDIR}/MyWizzard.o \
	${OBJECTDIR}/NTFSDrive.o \
	${OBJECTDIR}/Page_EndPage.o \
	${OBJECTDIR}/Page_GreetingPage.o \
	${OBJECTDIR}/Page_SelectDestinationDirPage.o \
	${OBJECTDIR}/Page_SelectFilesPage.o \
	${OBJECTDIR}/Page_SelectSourceDrivePage.o \
	${OBJECTDIR}/languages/LanguageDialog.o \
	${OBJECTDIR}/languages/LanguageFactory.o \
	${OBJECTDIR}/languages/LanguagesImpl.o \
	${OBJECTDIR}/main.o


# C Compiler Flags
CFLAGS=-D HAVE_STRING_H -D HAVE_STDLIB_H -D HAVE_TIME_H -D HAVE_SYS_STAT_H -D HAVE_WINDEF_H -D HAVE_WINDEF_H -D HAVE_WINBASE_H -D HAVE_WINDOWS_H -D IOCTL_STORAGE_QUERY_PROPERTY -D HAVE_DDK_NTDDSTOR_H -D TESTDISK_LSB

# CC Compiler Flags
CCFLAGS=-m32 -mwindows -Wall -O0 -D UNICODE
CXXFLAGS=-m32 -mwindows -Wall -O0 -D UNICODE

# Fortran Compiler Flags
FFLAGS=

# Assembler Flags
ASFLAGS=

# Link Libraries and Options
LDLIBSOPTIONS=-L../../wxWidgets-3.0.1/lib undelete.res -lwx_baseu-3.0.dll -lwx_baseu_net-3.0.dll -lwx_baseu_xml-3.0.dll -lwx_mswu_adv-3.0.dll -lwx_mswu_aui-3.0.dll -lwx_mswu_core-3.0.dll -lwx_mswu_gl-3.0.dll -lwx_mswu_html-3.0.dll -lwx_mswu_media-3.0.dll -lwx_mswu_propgrid-3.0.dll -lwx_mswu_qa-3.0.dll -lwx_mswu_ribbon-3.0.dll -lwx_mswu_richtext-3.0.dll -lwx_mswu_stc-3.0.dll -lwx_mswu_webview-3.0.dll -lwx_mswu_xrc-3.0.dll -lwxexpat-3.0 -lwxjpeg-3.0 -lwxpng-3.0 -lwxregexu-3.0 -lwxscintilla-3.0 -lwxtiff-3.0 -lwxzlib-3.0

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	"${MAKE}"  -f nbproject/Makefile-${CND_CONF}.mk ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/GetMyFilesBack.exe

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/GetMyFilesBack.exe: ${OBJECTFILES}
	${MKDIR} -p ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}
	${LINK.cc} -o ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/GetMyFilesBack ${OBJECTFILES} ${LDLIBSOPTIONS}

${OBJECTDIR}/CustomButton.o: CustomButton.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -I../../wxWidgets-3.0.1/lib/wx/include/msw-unicode-3.0 -I../../wxWidgets-3.0.1/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/CustomButton.o CustomButton.cpp

${OBJECTDIR}/FAT/addpart.o: FAT/addpart.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/addpart.o FAT/addpart.c

${OBJECTDIR}/FAT/analyse.o: FAT/analyse.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/analyse.o FAT/analyse.c

${OBJECTDIR}/FAT/askloc.o: FAT/askloc.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/askloc.o FAT/askloc.c

${OBJECTDIR}/FAT/autoset.o: FAT/autoset.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/autoset.o FAT/autoset.c

${OBJECTDIR}/FAT/bfs.o: FAT/bfs.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/bfs.o FAT/bfs.c

${OBJECTDIR}/FAT/bsd.o: FAT/bsd.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/bsd.o FAT/bsd.c

${OBJECTDIR}/FAT/btrfs.o: FAT/btrfs.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/btrfs.o FAT/btrfs.c

${OBJECTDIR}/FAT/chgtype.o: FAT/chgtype.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/chgtype.o FAT/chgtype.c

${OBJECTDIR}/FAT/chgtypen.o: FAT/chgtypen.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/chgtypen.o FAT/chgtypen.c

${OBJECTDIR}/FAT/common.o: FAT/common.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/common.o FAT/common.c

${OBJECTDIR}/FAT/cramfs.o: FAT/cramfs.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/cramfs.o FAT/cramfs.c

${OBJECTDIR}/FAT/crc.o: FAT/crc.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/crc.o FAT/crc.c

${OBJECTDIR}/FAT/dfxml.o: FAT/dfxml.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/dfxml.o FAT/dfxml.c

${OBJECTDIR}/FAT/dir.o: FAT/dir.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/dir.o FAT/dir.c

${OBJECTDIR}/FAT/ewf.o: FAT/ewf.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/ewf.o FAT/ewf.c

${OBJECTDIR}/FAT/exfat.o: FAT/exfat.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/exfat.o FAT/exfat.c

${OBJECTDIR}/FAT/exfatp.o: FAT/exfatp.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/exfatp.o FAT/exfatp.c

${OBJECTDIR}/FAT/ext2.o: FAT/ext2.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/ext2.o FAT/ext2.c

${OBJECTDIR}/FAT/ext2_dir.o: FAT/ext2_dir.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/ext2_dir.o FAT/ext2_dir.c

${OBJECTDIR}/FAT/ext2grp.o: FAT/ext2grp.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/ext2grp.o FAT/ext2grp.c

${OBJECTDIR}/FAT/ext2p.o: FAT/ext2p.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/ext2p.o FAT/ext2p.c

${OBJECTDIR}/FAT/fat.o: FAT/fat.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/fat.o FAT/fat.c

${OBJECTDIR}/FAT/fat_cluster.o: FAT/fat_cluster.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/fat_cluster.o FAT/fat_cluster.c

${OBJECTDIR}/FAT/fat_dir.o: FAT/fat_dir.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/fat_dir.o FAT/fat_dir.c

${OBJECTDIR}/FAT/fat_unformat.o: FAT/fat_unformat.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/fat_unformat.o FAT/fat_unformat.c

${OBJECTDIR}/FAT/fatp.o: FAT/fatp.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/fatp.o FAT/fatp.c

${OBJECTDIR}/FAT/fatx.o: FAT/fatx.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/fatx.o FAT/fatx.c

${OBJECTDIR}/FAT/file_1cd.o: FAT/file_1cd.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/file_1cd.o FAT/file_1cd.c

${OBJECTDIR}/FAT/file_7z.o: FAT/file_7z.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/file_7z.o FAT/file_7z.c

${OBJECTDIR}/FAT/file_DB.o: FAT/file_DB.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/file_DB.o FAT/file_DB.c

${OBJECTDIR}/FAT/file_a.o: FAT/file_a.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/file_a.o FAT/file_a.c

${OBJECTDIR}/FAT/file_ab.o: FAT/file_ab.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/file_ab.o FAT/file_ab.c

${OBJECTDIR}/FAT/file_abcdp.o: FAT/file_abcdp.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/file_abcdp.o FAT/file_abcdp.c

${OBJECTDIR}/FAT/file_abr.o: FAT/file_abr.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/file_abr.o FAT/file_abr.c

${OBJECTDIR}/FAT/file_acb.o: FAT/file_acb.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/file_acb.o FAT/file_acb.c

${OBJECTDIR}/FAT/file_ace.o: FAT/file_ace.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/file_ace.o FAT/file_ace.c

${OBJECTDIR}/FAT/file_ado.o: FAT/file_ado.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/file_ado.o FAT/file_ado.c

${OBJECTDIR}/FAT/file_ahn.o: FAT/file_ahn.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/file_ahn.o FAT/file_ahn.c

${OBJECTDIR}/FAT/file_aif.o: FAT/file_aif.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/file_aif.o FAT/file_aif.c

${OBJECTDIR}/FAT/file_all.o: FAT/file_all.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/file_all.o FAT/file_all.c

${OBJECTDIR}/FAT/file_als.o: FAT/file_als.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/file_als.o FAT/file_als.c

${OBJECTDIR}/FAT/file_amd.o: FAT/file_amd.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/file_amd.o FAT/file_amd.c

${OBJECTDIR}/FAT/file_amr.o: FAT/file_amr.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/file_amr.o FAT/file_amr.c

${OBJECTDIR}/FAT/file_apa.o: FAT/file_apa.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/file_apa.o FAT/file_apa.c

${OBJECTDIR}/FAT/file_ape.o: FAT/file_ape.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/file_ape.o FAT/file_ape.c

${OBJECTDIR}/FAT/file_apple.o: FAT/file_apple.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/file_apple.o FAT/file_apple.c

${OBJECTDIR}/FAT/file_arj.o: FAT/file_arj.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/file_arj.o FAT/file_arj.c

${OBJECTDIR}/FAT/file_asf.o: FAT/file_asf.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/file_asf.o FAT/file_asf.c

${OBJECTDIR}/FAT/file_asl.o: FAT/file_asl.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/file_asl.o FAT/file_asl.c

${OBJECTDIR}/FAT/file_asm.o: FAT/file_asm.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/file_asm.o FAT/file_asm.c

${OBJECTDIR}/FAT/file_atd.o: FAT/file_atd.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/file_atd.o FAT/file_atd.c

${OBJECTDIR}/FAT/file_au.o: FAT/file_au.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/file_au.o FAT/file_au.c

${OBJECTDIR}/FAT/file_axx.o: FAT/file_axx.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/file_axx.o FAT/file_axx.c

${OBJECTDIR}/FAT/file_bac.o: FAT/file_bac.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/file_bac.o FAT/file_bac.c

${OBJECTDIR}/FAT/file_bim.o: FAT/file_bim.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/file_bim.o FAT/file_bim.c

${OBJECTDIR}/FAT/file_binvox.o: FAT/file_binvox.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/file_binvox.o FAT/file_binvox.c

${OBJECTDIR}/FAT/file_bkf.o: FAT/file_bkf.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/file_bkf.o FAT/file_bkf.c

${OBJECTDIR}/FAT/file_bld.o: FAT/file_bld.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/file_bld.o FAT/file_bld.c

${OBJECTDIR}/FAT/file_bmp.o: FAT/file_bmp.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/file_bmp.o FAT/file_bmp.c

${OBJECTDIR}/FAT/file_bz2.o: FAT/file_bz2.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/file_bz2.o FAT/file_bz2.c

${OBJECTDIR}/FAT/file_cab.o: FAT/file_cab.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/file_cab.o FAT/file_cab.c

${OBJECTDIR}/FAT/file_caf.o: FAT/file_caf.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/file_caf.o FAT/file_caf.c

${OBJECTDIR}/FAT/file_cam.o: FAT/file_cam.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/file_cam.o FAT/file_cam.c

${OBJECTDIR}/FAT/file_catdrawing.o: FAT/file_catdrawing.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/file_catdrawing.o FAT/file_catdrawing.c

${OBJECTDIR}/FAT/file_cdt.o: FAT/file_cdt.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/file_cdt.o FAT/file_cdt.c

${OBJECTDIR}/FAT/file_chm.o: FAT/file_chm.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/file_chm.o FAT/file_chm.c

${OBJECTDIR}/FAT/file_class.o: FAT/file_class.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/file_class.o FAT/file_class.c

${OBJECTDIR}/FAT/file_cm.o: FAT/file_cm.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/file_cm.o FAT/file_cm.c

${OBJECTDIR}/FAT/file_compress.o: FAT/file_compress.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/file_compress.o FAT/file_compress.c

${OBJECTDIR}/FAT/file_cow.o: FAT/file_cow.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/file_cow.o FAT/file_cow.c

${OBJECTDIR}/FAT/file_crw.o: FAT/file_crw.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/file_crw.o FAT/file_crw.c

${OBJECTDIR}/FAT/file_csh.o: FAT/file_csh.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/file_csh.o FAT/file_csh.c

${OBJECTDIR}/FAT/file_ctg.o: FAT/file_ctg.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/file_ctg.o FAT/file_ctg.c

${OBJECTDIR}/FAT/file_cwk.o: FAT/file_cwk.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/file_cwk.o FAT/file_cwk.c

${OBJECTDIR}/FAT/file_d2s.o: FAT/file_d2s.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/file_d2s.o FAT/file_d2s.c

${OBJECTDIR}/FAT/file_dar.o: FAT/file_dar.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/file_dar.o FAT/file_dar.c

${OBJECTDIR}/FAT/file_dat.o: FAT/file_dat.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/file_dat.o FAT/file_dat.c

${OBJECTDIR}/FAT/file_dbf.o: FAT/file_dbf.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/file_dbf.o FAT/file_dbf.c

${OBJECTDIR}/FAT/file_dbn.o: FAT/file_dbn.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/file_dbn.o FAT/file_dbn.c

${OBJECTDIR}/FAT/file_ddf.o: FAT/file_ddf.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/file_ddf.o FAT/file_ddf.c

${OBJECTDIR}/FAT/file_dex.o: FAT/file_dex.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/file_dex.o FAT/file_dex.c

${OBJECTDIR}/FAT/file_dim.o: FAT/file_dim.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/file_dim.o FAT/file_dim.c

${OBJECTDIR}/FAT/file_dir.o: FAT/file_dir.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/file_dir.o FAT/file_dir.c

${OBJECTDIR}/FAT/file_djv.o: FAT/file_djv.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/file_djv.o FAT/file_djv.c

${OBJECTDIR}/FAT/file_dmp.o: FAT/file_dmp.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/file_dmp.o FAT/file_dmp.c

${OBJECTDIR}/FAT/file_doc.o: FAT/file_doc.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/file_doc.o FAT/file_doc.c

${OBJECTDIR}/FAT/file_dpx.o: FAT/file_dpx.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/file_dpx.o FAT/file_dpx.c

${OBJECTDIR}/FAT/file_drw.o: FAT/file_drw.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/file_drw.o FAT/file_drw.c

${OBJECTDIR}/FAT/file_ds2.o: FAT/file_ds2.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/file_ds2.o FAT/file_ds2.c

${OBJECTDIR}/FAT/file_dsc.o: FAT/file_dsc.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/file_dsc.o FAT/file_dsc.c

${OBJECTDIR}/FAT/file_dss.o: FAT/file_dss.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/file_dss.o FAT/file_dss.c

${OBJECTDIR}/FAT/file_dta.o: FAT/file_dta.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/file_dta.o FAT/file_dta.c

${OBJECTDIR}/FAT/file_dump.o: FAT/file_dump.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/file_dump.o FAT/file_dump.c

${OBJECTDIR}/FAT/file_dv.o: FAT/file_dv.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/file_dv.o FAT/file_dv.c

${OBJECTDIR}/FAT/file_dwg.o: FAT/file_dwg.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/file_dwg.o FAT/file_dwg.c

${OBJECTDIR}/FAT/file_dxf.o: FAT/file_dxf.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/file_dxf.o FAT/file_dxf.c

${OBJECTDIR}/FAT/file_e01.o: FAT/file_e01.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/file_e01.o FAT/file_e01.c

${OBJECTDIR}/FAT/file_ecryptfs.o: FAT/file_ecryptfs.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/file_ecryptfs.o FAT/file_ecryptfs.c

${OBJECTDIR}/FAT/file_edb.o: FAT/file_edb.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/file_edb.o FAT/file_edb.c

${OBJECTDIR}/FAT/file_elf.o: FAT/file_elf.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/file_elf.o FAT/file_elf.c

${OBJECTDIR}/FAT/file_emf.o: FAT/file_emf.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/file_emf.o FAT/file_emf.c

${OBJECTDIR}/FAT/file_evt.o: FAT/file_evt.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/file_evt.o FAT/file_evt.c

${OBJECTDIR}/FAT/file_exe.o: FAT/file_exe.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/file_exe.o FAT/file_exe.c

${OBJECTDIR}/FAT/file_exs.o: FAT/file_exs.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/file_exs.o FAT/file_exs.c

${OBJECTDIR}/FAT/file_ext.o: FAT/file_ext.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/file_ext.o FAT/file_ext.c

${OBJECTDIR}/FAT/file_ext2.o: FAT/file_ext2.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/file_ext2.o FAT/file_ext2.c

${OBJECTDIR}/FAT/file_fat.o: FAT/file_fat.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/file_fat.o FAT/file_fat.c

${OBJECTDIR}/FAT/file_fbf.o: FAT/file_fbf.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/file_fbf.o FAT/file_fbf.c

${OBJECTDIR}/FAT/file_fbk.o: FAT/file_fbk.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/file_fbk.o FAT/file_fbk.c

${OBJECTDIR}/FAT/file_fcp.o: FAT/file_fcp.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/file_fcp.o FAT/file_fcp.c

${OBJECTDIR}/FAT/file_fcs.o: FAT/file_fcs.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/file_fcs.o FAT/file_fcs.c

${OBJECTDIR}/FAT/file_fdb.o: FAT/file_fdb.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/file_fdb.o FAT/file_fdb.c

${OBJECTDIR}/FAT/file_fh10.o: FAT/file_fh10.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/file_fh10.o FAT/file_fh10.c

${OBJECTDIR}/FAT/file_fh5.o: FAT/file_fh5.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/file_fh5.o FAT/file_fh5.c

${OBJECTDIR}/FAT/file_filevault.o: FAT/file_filevault.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/file_filevault.o FAT/file_filevault.c

${OBJECTDIR}/FAT/file_fits.o: FAT/file_fits.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/file_fits.o FAT/file_fits.c

${OBJECTDIR}/FAT/file_flac.o: FAT/file_flac.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/file_flac.o FAT/file_flac.c

${OBJECTDIR}/FAT/file_flp.o: FAT/file_flp.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/file_flp.o FAT/file_flp.c

${OBJECTDIR}/FAT/file_flv.o: FAT/file_flv.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/file_flv.o FAT/file_flv.c

${OBJECTDIR}/FAT/file_fob.o: FAT/file_fob.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/file_fob.o FAT/file_fob.c

${OBJECTDIR}/FAT/file_found.o: FAT/file_found.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/file_found.o FAT/file_found.c

${OBJECTDIR}/FAT/file_fp5.o: FAT/file_fp5.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/file_fp5.o FAT/file_fp5.c

${OBJECTDIR}/FAT/file_fp7.o: FAT/file_fp7.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/file_fp7.o FAT/file_fp7.c

${OBJECTDIR}/FAT/file_freeway.o: FAT/file_freeway.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/file_freeway.o FAT/file_freeway.c

${OBJECTDIR}/FAT/file_frm.o: FAT/file_frm.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/file_frm.o FAT/file_frm.c

${OBJECTDIR}/FAT/file_fs.o: FAT/file_fs.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/file_fs.o FAT/file_fs.c

${OBJECTDIR}/FAT/file_fwd.o: FAT/file_fwd.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/file_fwd.o FAT/file_fwd.c

${OBJECTDIR}/FAT/file_gam.o: FAT/file_gam.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/file_gam.o FAT/file_gam.c

${OBJECTDIR}/FAT/file_gct.o: FAT/file_gct.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/file_gct.o FAT/file_gct.c

${OBJECTDIR}/FAT/file_gho.o: FAT/file_gho.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/file_gho.o FAT/file_gho.c

${OBJECTDIR}/FAT/file_gif.o: FAT/file_gif.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/file_gif.o FAT/file_gif.c

${OBJECTDIR}/FAT/file_gm6.o: FAT/file_gm6.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/file_gm6.o FAT/file_gm6.c

${OBJECTDIR}/FAT/file_gp5.o: FAT/file_gp5.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/file_gp5.o FAT/file_gp5.c

${OBJECTDIR}/FAT/file_gpg.o: FAT/file_gpg.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/file_gpg.o FAT/file_gpg.c

${OBJECTDIR}/FAT/file_gz.o: FAT/file_gz.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/file_gz.o FAT/file_gz.c

${OBJECTDIR}/FAT/file_hdf.o: FAT/file_hdf.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/file_hdf.o FAT/file_hdf.c

${OBJECTDIR}/FAT/file_hds.o: FAT/file_hds.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/file_hds.o FAT/file_hds.c

${OBJECTDIR}/FAT/file_hfsp.o: FAT/file_hfsp.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/file_hfsp.o FAT/file_hfsp.c

${OBJECTDIR}/FAT/file_hr9.o: FAT/file_hr9.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/file_hr9.o FAT/file_hr9.c

${OBJECTDIR}/FAT/file_http.o: FAT/file_http.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/file_http.o FAT/file_http.c

${OBJECTDIR}/FAT/file_icc.o: FAT/file_icc.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/file_icc.o FAT/file_icc.c

${OBJECTDIR}/FAT/file_ico.o: FAT/file_ico.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/file_ico.o FAT/file_ico.c

${OBJECTDIR}/FAT/file_ifo.o: FAT/file_ifo.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/file_ifo.o FAT/file_ifo.c

${OBJECTDIR}/FAT/file_imb.o: FAT/file_imb.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/file_imb.o FAT/file_imb.c

${OBJECTDIR}/FAT/file_indd.o: FAT/file_indd.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/file_indd.o FAT/file_indd.c

${OBJECTDIR}/FAT/file_info.o: FAT/file_info.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/file_info.o FAT/file_info.c

${OBJECTDIR}/FAT/file_iso.o: FAT/file_iso.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/file_iso.o FAT/file_iso.c

${OBJECTDIR}/FAT/file_it.o: FAT/file_it.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/file_it.o FAT/file_it.c

${OBJECTDIR}/FAT/file_itu.o: FAT/file_itu.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/file_itu.o FAT/file_itu.c

${OBJECTDIR}/FAT/file_jpg.o: FAT/file_jpg.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/file_jpg.o FAT/file_jpg.c

${OBJECTDIR}/FAT/file_kdb.o: FAT/file_kdb.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/file_kdb.o FAT/file_kdb.c

${OBJECTDIR}/FAT/file_kdbx.o: FAT/file_kdbx.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/file_kdbx.o FAT/file_kdbx.c

${OBJECTDIR}/FAT/file_ldf.o: FAT/file_ldf.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/file_ldf.o FAT/file_ldf.c

${OBJECTDIR}/FAT/file_list.o: FAT/file_list.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/file_list.o FAT/file_list.c

${OBJECTDIR}/FAT/file_lit.o: FAT/file_lit.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/file_lit.o FAT/file_lit.c

${OBJECTDIR}/FAT/file_lnk.o: FAT/file_lnk.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/file_lnk.o FAT/file_lnk.c

${OBJECTDIR}/FAT/file_logic.o: FAT/file_logic.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/file_logic.o FAT/file_logic.c

${OBJECTDIR}/FAT/file_lso.o: FAT/file_lso.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/file_lso.o FAT/file_lso.c

${OBJECTDIR}/FAT/file_lxo.o: FAT/file_lxo.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/file_lxo.o FAT/file_lxo.c

${OBJECTDIR}/FAT/file_lzo.o: FAT/file_lzo.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/file_lzo.o FAT/file_lzo.c

${OBJECTDIR}/FAT/file_m2ts.o: FAT/file_m2ts.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/file_m2ts.o FAT/file_m2ts.c

${OBJECTDIR}/FAT/file_mat.o: FAT/file_mat.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/file_mat.o FAT/file_mat.c

${OBJECTDIR}/FAT/file_max.o: FAT/file_max.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/file_max.o FAT/file_max.c

${OBJECTDIR}/FAT/file_mb.o: FAT/file_mb.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/file_mb.o FAT/file_mb.c

${OBJECTDIR}/FAT/file_mcd.o: FAT/file_mcd.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/file_mcd.o FAT/file_mcd.c

${OBJECTDIR}/FAT/file_mdb.o: FAT/file_mdb.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/file_mdb.o FAT/file_mdb.c

${OBJECTDIR}/FAT/file_mdf.o: FAT/file_mdf.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/file_mdf.o FAT/file_mdf.c

${OBJECTDIR}/FAT/file_mfa.o: FAT/file_mfa.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/file_mfa.o FAT/file_mfa.c

${OBJECTDIR}/FAT/file_mfg.o: FAT/file_mfg.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/file_mfg.o FAT/file_mfg.c

${OBJECTDIR}/FAT/file_mft.o: FAT/file_mft.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/file_mft.o FAT/file_mft.c

${OBJECTDIR}/FAT/file_mid.o: FAT/file_mid.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/file_mid.o FAT/file_mid.c

${OBJECTDIR}/FAT/file_mig.o: FAT/file_mig.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/file_mig.o FAT/file_mig.c

${OBJECTDIR}/FAT/file_mk5.o: FAT/file_mk5.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/file_mk5.o FAT/file_mk5.c

${OBJECTDIR}/FAT/file_mkv.o: FAT/file_mkv.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/file_mkv.o FAT/file_mkv.c

${OBJECTDIR}/FAT/file_mobi.o: FAT/file_mobi.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/file_mobi.o FAT/file_mobi.c

${OBJECTDIR}/FAT/file_mov.o: FAT/file_mov.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/file_mov.o FAT/file_mov.c

${OBJECTDIR}/FAT/file_mp3.o: FAT/file_mp3.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/file_mp3.o FAT/file_mp3.c

${OBJECTDIR}/FAT/file_mpg.o: FAT/file_mpg.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/file_mpg.o FAT/file_mpg.c

${OBJECTDIR}/FAT/file_mrw.o: FAT/file_mrw.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/file_mrw.o FAT/file_mrw.c

${OBJECTDIR}/FAT/file_mus.o: FAT/file_mus.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/file_mus.o FAT/file_mus.c

${OBJECTDIR}/FAT/file_mxf.o: FAT/file_mxf.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/file_mxf.o FAT/file_mxf.c

${OBJECTDIR}/FAT/file_myo.o: FAT/file_myo.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/file_myo.o FAT/file_myo.c

${OBJECTDIR}/FAT/file_mysql.o: FAT/file_mysql.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/file_mysql.o FAT/file_mysql.c

${OBJECTDIR}/FAT/file_nds.o: FAT/file_nds.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/file_nds.o FAT/file_nds.c

${OBJECTDIR}/FAT/file_njx.o: FAT/file_njx.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/file_njx.o FAT/file_njx.c

${OBJECTDIR}/FAT/file_nk2.o: FAT/file_nk2.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/file_nk2.o FAT/file_nk2.c

${OBJECTDIR}/FAT/file_nsf.o: FAT/file_nsf.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/file_nsf.o FAT/file_nsf.c

${OBJECTDIR}/FAT/file_oci.o: FAT/file_oci.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/file_oci.o FAT/file_oci.c

${OBJECTDIR}/FAT/file_ogg.o: FAT/file_ogg.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/file_ogg.o FAT/file_ogg.c

${OBJECTDIR}/FAT/file_one.o: FAT/file_one.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/file_one.o FAT/file_one.c

${OBJECTDIR}/FAT/file_orf.o: FAT/file_orf.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/file_orf.o FAT/file_orf.c

${OBJECTDIR}/FAT/file_paf.o: FAT/file_paf.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/file_paf.o FAT/file_paf.c

${OBJECTDIR}/FAT/file_pap.o: FAT/file_pap.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/file_pap.o FAT/file_pap.c

${OBJECTDIR}/FAT/file_par2.o: FAT/file_par2.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/file_par2.o FAT/file_par2.c

${OBJECTDIR}/FAT/file_pcap.o: FAT/file_pcap.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/file_pcap.o FAT/file_pcap.c

${OBJECTDIR}/FAT/file_pct.o: FAT/file_pct.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/file_pct.o FAT/file_pct.c

${OBJECTDIR}/FAT/file_pcx.o: FAT/file_pcx.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/file_pcx.o FAT/file_pcx.c

${OBJECTDIR}/FAT/file_pdf.o: FAT/file_pdf.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/file_pdf.o FAT/file_pdf.c

${OBJECTDIR}/FAT/file_pds.o: FAT/file_pds.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/file_pds.o FAT/file_pds.c

${OBJECTDIR}/FAT/file_pfx.o: FAT/file_pfx.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/file_pfx.o FAT/file_pfx.c

${OBJECTDIR}/FAT/file_plt.o: FAT/file_plt.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/file_plt.o FAT/file_plt.c

${OBJECTDIR}/FAT/file_png.o: FAT/file_png.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/file_png.o FAT/file_png.c

${OBJECTDIR}/FAT/file_pnm.o: FAT/file_pnm.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/file_pnm.o FAT/file_pnm.c

${OBJECTDIR}/FAT/file_prc.o: FAT/file_prc.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/file_prc.o FAT/file_prc.c

${OBJECTDIR}/FAT/file_prt.o: FAT/file_prt.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/file_prt.o FAT/file_prt.c

${OBJECTDIR}/FAT/file_ps.o: FAT/file_ps.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/file_ps.o FAT/file_ps.c

${OBJECTDIR}/FAT/file_psd.o: FAT/file_psd.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/file_psd.o FAT/file_psd.c

${OBJECTDIR}/FAT/file_psf.o: FAT/file_psf.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/file_psf.o FAT/file_psf.c

${OBJECTDIR}/FAT/file_psp.o: FAT/file_psp.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/file_psp.o FAT/file_psp.c

${OBJECTDIR}/FAT/file_pst.o: FAT/file_pst.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/file_pst.o FAT/file_pst.c

${OBJECTDIR}/FAT/file_ptb.o: FAT/file_ptb.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/file_ptb.o FAT/file_ptb.c

${OBJECTDIR}/FAT/file_ptf.o: FAT/file_ptf.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/file_ptf.o FAT/file_ptf.c

${OBJECTDIR}/FAT/file_pyc.o: FAT/file_pyc.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/file_pyc.o FAT/file_pyc.c

${OBJECTDIR}/FAT/file_pzf.o: FAT/file_pzf.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/file_pzf.o FAT/file_pzf.c

${OBJECTDIR}/FAT/file_pzh.o: FAT/file_pzh.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/file_pzh.o FAT/file_pzh.c

${OBJECTDIR}/FAT/file_qbb.o: FAT/file_qbb.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/file_qbb.o FAT/file_qbb.c

${OBJECTDIR}/FAT/file_qdf.o: FAT/file_qdf.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/file_qdf.o FAT/file_qdf.c

${OBJECTDIR}/FAT/file_qkt.o: FAT/file_qkt.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/file_qkt.o FAT/file_qkt.c

${OBJECTDIR}/FAT/file_qxd.o: FAT/file_qxd.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/file_qxd.o FAT/file_qxd.c

${OBJECTDIR}/FAT/file_r3d.o: FAT/file_r3d.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/file_r3d.o FAT/file_r3d.c

${OBJECTDIR}/FAT/file_ra.o: FAT/file_ra.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/file_ra.o FAT/file_ra.c

${OBJECTDIR}/FAT/file_raf.o: FAT/file_raf.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/file_raf.o FAT/file_raf.c

${OBJECTDIR}/FAT/file_rar.o: FAT/file_rar.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/file_rar.o FAT/file_rar.c

${OBJECTDIR}/FAT/file_raw.o: FAT/file_raw.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/file_raw.o FAT/file_raw.c

${OBJECTDIR}/FAT/file_rdc.o: FAT/file_rdc.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/file_rdc.o FAT/file_rdc.c

${OBJECTDIR}/FAT/file_reg.o: FAT/file_reg.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/file_reg.o FAT/file_reg.c

${OBJECTDIR}/FAT/file_res.o: FAT/file_res.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/file_res.o FAT/file_res.c

${OBJECTDIR}/FAT/file_rfp.o: FAT/file_rfp.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/file_rfp.o FAT/file_rfp.c

${OBJECTDIR}/FAT/file_riff.o: FAT/file_riff.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/file_riff.o FAT/file_riff.c

${OBJECTDIR}/FAT/file_rm.o: FAT/file_rm.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/file_rm.o FAT/file_rm.c

${OBJECTDIR}/FAT/file_rns.o: FAT/file_rns.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/file_rns.o FAT/file_rns.c

${OBJECTDIR}/FAT/file_rpm.o: FAT/file_rpm.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/file_rpm.o FAT/file_rpm.c

${OBJECTDIR}/FAT/file_rw2.o: FAT/file_rw2.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/file_rw2.o FAT/file_rw2.c

${OBJECTDIR}/FAT/file_rx2.o: FAT/file_rx2.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/file_rx2.o FAT/file_rx2.c

${OBJECTDIR}/FAT/file_save.o: FAT/file_save.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/file_save.o FAT/file_save.c

${OBJECTDIR}/FAT/file_ses.o: FAT/file_ses.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/file_ses.o FAT/file_ses.c

${OBJECTDIR}/FAT/file_sib.o: FAT/file_sib.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/file_sib.o FAT/file_sib.c

${OBJECTDIR}/FAT/file_sig.o: FAT/file_sig.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/file_sig.o FAT/file_sig.c

${OBJECTDIR}/FAT/file_sit.o: FAT/file_sit.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/file_sit.o FAT/file_sit.c

${OBJECTDIR}/FAT/file_skd.o: FAT/file_skd.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/file_skd.o FAT/file_skd.c

${OBJECTDIR}/FAT/file_skp.o: FAT/file_skp.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/file_skp.o FAT/file_skp.c

${OBJECTDIR}/FAT/file_sp3.o: FAT/file_sp3.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/file_sp3.o FAT/file_sp3.c

${OBJECTDIR}/FAT/file_spe.o: FAT/file_spe.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/file_spe.o FAT/file_spe.c

${OBJECTDIR}/FAT/file_spf.o: FAT/file_spf.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/file_spf.o FAT/file_spf.c

${OBJECTDIR}/FAT/file_spss.o: FAT/file_spss.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/file_spss.o FAT/file_spss.c

${OBJECTDIR}/FAT/file_sql.o: FAT/file_sql.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/file_sql.o FAT/file_sql.c

${OBJECTDIR}/FAT/file_sqm.o: FAT/file_sqm.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/file_sqm.o FAT/file_sqm.c

${OBJECTDIR}/FAT/file_stl.o: FAT/file_stl.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/file_stl.o FAT/file_stl.c

${OBJECTDIR}/FAT/file_stu.o: FAT/file_stu.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/file_stu.o FAT/file_stu.c

${OBJECTDIR}/FAT/file_swf.o: FAT/file_swf.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/file_swf.o FAT/file_swf.c

${OBJECTDIR}/FAT/file_tar.o: FAT/file_tar.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/file_tar.o FAT/file_tar.c

${OBJECTDIR}/FAT/file_tax.o: FAT/file_tax.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/file_tax.o FAT/file_tax.c

${OBJECTDIR}/FAT/file_tib.o: FAT/file_tib.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/file_tib.o FAT/file_tib.c

${OBJECTDIR}/FAT/file_tiff.o: FAT/file_tiff.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/file_tiff.o FAT/file_tiff.c

${OBJECTDIR}/FAT/file_tivo.o: FAT/file_tivo.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/file_tivo.o FAT/file_tivo.c

${OBJECTDIR}/FAT/file_torrent.o: FAT/file_torrent.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/file_torrent.o FAT/file_torrent.c

${OBJECTDIR}/FAT/file_tph.o: FAT/file_tph.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/file_tph.o FAT/file_tph.c

${OBJECTDIR}/FAT/file_tpl.o: FAT/file_tpl.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/file_tpl.o FAT/file_tpl.c

${OBJECTDIR}/FAT/file_ttf.o: FAT/file_ttf.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/file_ttf.o FAT/file_ttf.c

${OBJECTDIR}/FAT/file_txt.o: FAT/file_txt.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/file_txt.o FAT/file_txt.c

${OBJECTDIR}/FAT/file_tz.o: FAT/file_tz.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/file_tz.o FAT/file_tz.c

${OBJECTDIR}/FAT/file_v2i.o: FAT/file_v2i.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/file_v2i.o FAT/file_v2i.c

${OBJECTDIR}/FAT/file_vault.o: FAT/file_vault.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/file_vault.o FAT/file_vault.c

${OBJECTDIR}/FAT/file_vdi.o: FAT/file_vdi.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/file_vdi.o FAT/file_vdi.c

${OBJECTDIR}/FAT/file_veg.o: FAT/file_veg.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/file_veg.o FAT/file_veg.c

${OBJECTDIR}/FAT/file_vfb.o: FAT/file_vfb.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/file_vfb.o FAT/file_vfb.c

${OBJECTDIR}/FAT/file_vmdk.o: FAT/file_vmdk.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/file_vmdk.o FAT/file_vmdk.c

${OBJECTDIR}/FAT/file_vmg.o: FAT/file_vmg.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/file_vmg.o FAT/file_vmg.c

${OBJECTDIR}/FAT/file_wdp.o: FAT/file_wdp.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/file_wdp.o FAT/file_wdp.c

${OBJECTDIR}/FAT/file_win.o: FAT/file_win.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/file_win.o FAT/file_win.c

${OBJECTDIR}/FAT/file_wks.o: FAT/file_wks.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/file_wks.o FAT/file_wks.c

${OBJECTDIR}/FAT/file_wmf.o: FAT/file_wmf.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/file_wmf.o FAT/file_wmf.c

${OBJECTDIR}/FAT/file_wnk.o: FAT/file_wnk.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/file_wnk.o FAT/file_wnk.c

${OBJECTDIR}/FAT/file_wpb.o: FAT/file_wpb.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/file_wpb.o FAT/file_wpb.c

${OBJECTDIR}/FAT/file_wpd.o: FAT/file_wpd.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/file_wpd.o FAT/file_wpd.c

${OBJECTDIR}/FAT/file_wtv.o: FAT/file_wtv.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/file_wtv.o FAT/file_wtv.c

${OBJECTDIR}/FAT/file_wv.o: FAT/file_wv.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/file_wv.o FAT/file_wv.c

${OBJECTDIR}/FAT/file_x3f.o: FAT/file_x3f.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/file_x3f.o FAT/file_x3f.c

${OBJECTDIR}/FAT/file_xcf.o: FAT/file_xcf.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/file_xcf.o FAT/file_xcf.c

${OBJECTDIR}/FAT/file_xfi.o: FAT/file_xfi.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/file_xfi.o FAT/file_xfi.c

${OBJECTDIR}/FAT/file_xm.o: FAT/file_xm.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/file_xm.o FAT/file_xm.c

${OBJECTDIR}/FAT/file_xpt.o: FAT/file_xpt.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/file_xpt.o FAT/file_xpt.c

${OBJECTDIR}/FAT/file_xsv.o: FAT/file_xsv.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/file_xsv.o FAT/file_xsv.c

${OBJECTDIR}/FAT/file_xv.o: FAT/file_xv.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/file_xv.o FAT/file_xv.c

${OBJECTDIR}/FAT/file_xz.o: FAT/file_xz.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/file_xz.o FAT/file_xz.c

${OBJECTDIR}/FAT/file_zip.o: FAT/file_zip.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/file_zip.o FAT/file_zip.c

${OBJECTDIR}/FAT/filegen.o: FAT/filegen.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/filegen.o FAT/filegen.c

${OBJECTDIR}/FAT/fnctdsk.o: FAT/fnctdsk.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/fnctdsk.o FAT/fnctdsk.c

${OBJECTDIR}/FAT/geometry.o: FAT/geometry.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/geometry.o FAT/geometry.c

${OBJECTDIR}/FAT/gfs2.o: FAT/gfs2.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/gfs2.o FAT/gfs2.c

${OBJECTDIR}/FAT/hdaccess.o: FAT/hdaccess.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/hdaccess.o FAT/hdaccess.c

${OBJECTDIR}/FAT/hdcache.o: FAT/hdcache.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/hdcache.o FAT/hdcache.c

${OBJECTDIR}/FAT/hdwin32.o: FAT/hdwin32.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/hdwin32.o FAT/hdwin32.c

${OBJECTDIR}/FAT/hfs.o: FAT/hfs.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/hfs.o FAT/hfs.c

${OBJECTDIR}/FAT/hfsp.o: FAT/hfsp.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/hfsp.o FAT/hfsp.c

${OBJECTDIR}/FAT/hidden.o: FAT/hidden.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/hidden.o FAT/hidden.c

${OBJECTDIR}/FAT/hiddenn.o: FAT/hiddenn.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/hiddenn.o FAT/hiddenn.c

${OBJECTDIR}/FAT/hpa_dco.o: FAT/hpa_dco.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/hpa_dco.o FAT/hpa_dco.c

${OBJECTDIR}/FAT/hpfs.o: FAT/hpfs.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/hpfs.o FAT/hpfs.c

${OBJECTDIR}/FAT/intrf.o: FAT/intrf.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/intrf.o FAT/intrf.c

${OBJECTDIR}/FAT/intrfn.o: FAT/intrfn.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/intrfn.o FAT/intrfn.c

${OBJECTDIR}/FAT/iso.o: FAT/iso.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/iso.o FAT/iso.c

${OBJECTDIR}/FAT/jfs.o: FAT/jfs.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/jfs.o FAT/jfs.c

${OBJECTDIR}/FAT/list.o: FAT/list.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/list.o FAT/list.c

${OBJECTDIR}/FAT/list_sort.o: FAT/list_sort.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/list_sort.o FAT/list_sort.c

${OBJECTDIR}/FAT/log.o: FAT/log.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/log.o FAT/log.c

${OBJECTDIR}/FAT/log_part.o: FAT/log_part.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/log_part.o FAT/log_part.c

${OBJECTDIR}/FAT/luks.o: FAT/luks.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/luks.o FAT/luks.c

${OBJECTDIR}/FAT/lvm.o: FAT/lvm.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/lvm.o FAT/lvm.c

${OBJECTDIR}/FAT/md.o: FAT/md.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/md.o FAT/md.c

${OBJECTDIR}/FAT/misc.o: FAT/misc.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/misc.o FAT/misc.c

${OBJECTDIR}/FAT/netware.o: FAT/netware.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/netware.o FAT/netware.c

${OBJECTDIR}/FAT/nodisk.o: FAT/nodisk.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/nodisk.o FAT/nodisk.c

${OBJECTDIR}/FAT/ntfs.o: FAT/ntfs.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/ntfs.o FAT/ntfs.c

${OBJECTDIR}/FAT/ntfs_dir.o: FAT/ntfs_dir.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/ntfs_dir.o FAT/ntfs_dir.c

${OBJECTDIR}/FAT/ntfs_utl.o: FAT/ntfs_utl.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/ntfs_utl.o FAT/ntfs_utl.c

${OBJECTDIR}/FAT/ntfsp.o: FAT/ntfsp.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/ntfsp.o FAT/ntfsp.c

${OBJECTDIR}/FAT/partauto.o: FAT/partauto.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/partauto.o FAT/partauto.c

${OBJECTDIR}/FAT/partgpt.o: FAT/partgpt.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/partgpt.o FAT/partgpt.c

${OBJECTDIR}/FAT/partgptn.o: FAT/partgptn.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/partgptn.o FAT/partgptn.c

${OBJECTDIR}/FAT/partgptro.o: FAT/partgptro.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/partgptro.o FAT/partgptro.c

${OBJECTDIR}/FAT/parthumax.o: FAT/parthumax.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/parthumax.o FAT/parthumax.c

${OBJECTDIR}/FAT/parti386.o: FAT/parti386.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/parti386.o FAT/parti386.c

${OBJECTDIR}/FAT/parti386n.o: FAT/parti386n.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/parti386n.o FAT/parti386n.c

${OBJECTDIR}/FAT/partmac.o: FAT/partmac.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/partmac.o FAT/partmac.c

${OBJECTDIR}/FAT/partmacn.o: FAT/partmacn.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/partmacn.o FAT/partmacn.c

${OBJECTDIR}/FAT/partnone.o: FAT/partnone.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/partnone.o FAT/partnone.c

${OBJECTDIR}/FAT/partsun.o: FAT/partsun.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/partsun.o FAT/partsun.c

${OBJECTDIR}/FAT/partsunn.o: FAT/partsunn.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/partsunn.o FAT/partsunn.c

${OBJECTDIR}/FAT/partxbox.o: FAT/partxbox.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/partxbox.o FAT/partxbox.c

${OBJECTDIR}/FAT/partxboxn.o: FAT/partxboxn.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/partxboxn.o FAT/partxboxn.c

${OBJECTDIR}/FAT/pblocksize.o: FAT/pblocksize.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/pblocksize.o FAT/pblocksize.c

${OBJECTDIR}/FAT/pdisksel.o: FAT/pdisksel.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/pdisksel.o FAT/pdisksel.c

${OBJECTDIR}/FAT/pfree_whole.o: FAT/pfree_whole.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/pfree_whole.o FAT/pfree_whole.c

${OBJECTDIR}/FAT/phbf.o: FAT/phbf.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/phbf.o FAT/phbf.c

${OBJECTDIR}/FAT/phbs.o: FAT/phbs.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/phbs.o FAT/phbs.c

${OBJECTDIR}/FAT/phcfg.o: FAT/phcfg.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/phcfg.o FAT/phcfg.c

${OBJECTDIR}/FAT/phmain.o: FAT/phmain.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/phmain.o FAT/phmain.c

${OBJECTDIR}/FAT/phnc.o: FAT/phnc.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/phnc.o FAT/phnc.c

${OBJECTDIR}/FAT/photorec.o: FAT/photorec.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/photorec.o FAT/photorec.c

${OBJECTDIR}/FAT/phrecn.o: FAT/phrecn.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/phrecn.o FAT/phrecn.c

${OBJECTDIR}/FAT/ppartsel.o: FAT/ppartsel.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/ppartsel.o FAT/ppartsel.c

${OBJECTDIR}/FAT/rfs.o: FAT/rfs.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/rfs.o FAT/rfs.c

${OBJECTDIR}/FAT/savehdr.o: FAT/savehdr.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/savehdr.o FAT/savehdr.c

${OBJECTDIR}/FAT/sessionp.o: FAT/sessionp.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/sessionp.o FAT/sessionp.c

${OBJECTDIR}/FAT/setdate.o: FAT/setdate.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/setdate.o FAT/setdate.c

${OBJECTDIR}/FAT/sudo.o: FAT/sudo.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/sudo.o FAT/sudo.c

${OBJECTDIR}/FAT/sun.o: FAT/sun.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/sun.o FAT/sun.c

${OBJECTDIR}/FAT/swap.o: FAT/swap.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/swap.o FAT/swap.c

${OBJECTDIR}/FAT/sysv.o: FAT/sysv.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/sysv.o FAT/sysv.c

${OBJECTDIR}/FAT/ufs.o: FAT/ufs.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/ufs.o FAT/ufs.c

${OBJECTDIR}/FAT/unicode.o: FAT/unicode.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/unicode.o FAT/unicode.c

${OBJECTDIR}/FAT/vmfs.o: FAT/vmfs.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/vmfs.o FAT/vmfs.c

${OBJECTDIR}/FAT/wbfs.o: FAT/wbfs.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/wbfs.o FAT/wbfs.c

${OBJECTDIR}/FAT/win32.o: FAT/win32.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/win32.o FAT/win32.c

${OBJECTDIR}/FAT/xfs.o: FAT/xfs.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/xfs.o FAT/xfs.c

${OBJECTDIR}/FAT/zfs.o: FAT/zfs.c 
	${MKDIR} -p ${OBJECTDIR}/FAT
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FAT/zfs.o FAT/zfs.c

${OBJECTDIR}/IsValidFileName.o: IsValidFileName.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -I../../wxWidgets-3.0.1/lib/wx/include/msw-unicode-3.0 -I../../wxWidgets-3.0.1/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/IsValidFileName.o IsValidFileName.cpp

${OBJECTDIR}/MFTRecord.o: MFTRecord.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -I../../wxWidgets-3.0.1/lib/wx/include/msw-unicode-3.0 -I../../wxWidgets-3.0.1/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/MFTRecord.o MFTRecord.cpp

${OBJECTDIR}/MyThreadClass.o: MyThreadClass.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -I../../wxWidgets-3.0.1/lib/wx/include/msw-unicode-3.0 -I../../wxWidgets-3.0.1/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/MyThreadClass.o MyThreadClass.cpp

${OBJECTDIR}/MyWizzard.o: MyWizzard.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -I../../wxWidgets-3.0.1/lib/wx/include/msw-unicode-3.0 -I../../wxWidgets-3.0.1/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/MyWizzard.o MyWizzard.cpp

${OBJECTDIR}/NTFSDrive.o: NTFSDrive.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -I../../wxWidgets-3.0.1/lib/wx/include/msw-unicode-3.0 -I../../wxWidgets-3.0.1/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/NTFSDrive.o NTFSDrive.cpp

${OBJECTDIR}/Page_EndPage.o: Page_EndPage.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -I../../wxWidgets-3.0.1/lib/wx/include/msw-unicode-3.0 -I../../wxWidgets-3.0.1/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Page_EndPage.o Page_EndPage.cpp

${OBJECTDIR}/Page_GreetingPage.o: Page_GreetingPage.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -I../../wxWidgets-3.0.1/lib/wx/include/msw-unicode-3.0 -I../../wxWidgets-3.0.1/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Page_GreetingPage.o Page_GreetingPage.cpp

${OBJECTDIR}/Page_SelectDestinationDirPage.o: Page_SelectDestinationDirPage.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -I../../wxWidgets-3.0.1/lib/wx/include/msw-unicode-3.0 -I../../wxWidgets-3.0.1/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Page_SelectDestinationDirPage.o Page_SelectDestinationDirPage.cpp

${OBJECTDIR}/Page_SelectFilesPage.o: Page_SelectFilesPage.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -I../../wxWidgets-3.0.1/lib/wx/include/msw-unicode-3.0 -I../../wxWidgets-3.0.1/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Page_SelectFilesPage.o Page_SelectFilesPage.cpp

${OBJECTDIR}/Page_SelectSourceDrivePage.o: Page_SelectSourceDrivePage.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -I../../wxWidgets-3.0.1/lib/wx/include/msw-unicode-3.0 -I../../wxWidgets-3.0.1/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Page_SelectSourceDrivePage.o Page_SelectSourceDrivePage.cpp

${OBJECTDIR}/languages/LanguageDialog.o: languages/LanguageDialog.cpp 
	${MKDIR} -p ${OBJECTDIR}/languages
	${RM} "$@.d"
	$(COMPILE.cc) -g -I../../wxWidgets-3.0.1/lib/wx/include/msw-unicode-3.0 -I../../wxWidgets-3.0.1/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/languages/LanguageDialog.o languages/LanguageDialog.cpp

${OBJECTDIR}/languages/LanguageFactory.o: languages/LanguageFactory.cpp 
	${MKDIR} -p ${OBJECTDIR}/languages
	${RM} "$@.d"
	$(COMPILE.cc) -g -I../../wxWidgets-3.0.1/lib/wx/include/msw-unicode-3.0 -I../../wxWidgets-3.0.1/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/languages/LanguageFactory.o languages/LanguageFactory.cpp

${OBJECTDIR}/languages/LanguagesImpl.o: languages/LanguagesImpl.cpp 
	${MKDIR} -p ${OBJECTDIR}/languages
	${RM} "$@.d"
	$(COMPILE.cc) -g -I../../wxWidgets-3.0.1/lib/wx/include/msw-unicode-3.0 -I../../wxWidgets-3.0.1/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/languages/LanguagesImpl.o languages/LanguagesImpl.cpp

${OBJECTDIR}/main.o: main.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -I../../wxWidgets-3.0.1/lib/wx/include/msw-unicode-3.0 -I../../wxWidgets-3.0.1/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/main.o main.cpp

# Subprojects
.build-subprojects:

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r ${CND_BUILDDIR}/${CND_CONF}
	${RM} ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/GetMyFilesBack.exe

# Subprojects
.clean-subprojects:

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc
