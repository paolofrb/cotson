#!/bin/bash
#
# AUTHOR:       : SV (viola@dii.unisi.it)
# TITLE         : bootstrap.sh
# LICENSE       : MIT
# VERSION       : 0.0.2
# DATE          : 20160509
# NOTES         : NULL
#
#===============================================================================
#
# PURPOSE: Prepare the system for the COTSon installation
#


### NOTE ###
# echo "${BASH_LINENO[$FUNCNAME]}"
# echo "$BASH_SOURCE"
# echo "$LINENO"
### #### ###

### GLOBAL VARIABLES DEFINITIONS ###
SIMNOW_SUPPORTED_VER="4.6.2pub"
USERNAME=$(logname)
DIST_IMAGE="karmic64"
DIST=""
DIST1=""
VER=""
VERNUM=""
DEBUG=0
SIMNOW_DIR="" # ENV VARIABLE NEED elimina
g_simnow_dir=""
root=$(pwd)
VERBOSE=0

###
trap "exit 1" TERM
export TOP_PID=$$
###

DEBIAN_BASED_PKGS="g++ g++-multilib subversion genisoimage bison flex vnc4server  \
rxvt xfwm4 xfonts-100dpi xfonts-75dpi zsh sharutils build-essential xvnc4viewer screen \
liblua5.1-0 liblua5.1-0-dev zlib1g-dev indent xutils-dev libsqlite3-dev \
sqlite3 libdbd-sqlite3-perl libdbd-pg-perl gnuplot libboost-dev libboost-thread-dev lzma \
libxcursor1 libxrender1 libsm6 libxi6 libfontconfig1 "

FEDORA_BASED_PKGS="gcc gcc-c++ make subversion genisoimage bison flex ruby rubygems rxvt zsh sharutils screen \
gnuplot indent zlib-devel imake xorg-x11-utils ruby-libs openssl \
rxvt flex lua* lua-devel libpqxx* boost-devel boost-thread sqlite-devel sqlite lzma \
perl-Class-DBI-SQLite xorg-x11-fonts-100dpi.noarch xorg-x11-fonts-75dpi \
tigervnc tigervnc-server tigervnc-server-module xorg-x11-twm glibc-devel.i686 libstdc++.i686 \
glibc-static glibc-devel libstdc++ libtool svn flex "

DEBIAN_SUPPORTED_VERS="lenny | squeeze | wheezy | jessie"
FEDORA_SUPPORTED_VERS="Werewolf | Leonidas | Goddard | Laughlin | Lovelock | Verne | BeefyMiracle | SphericalCow | Schrödinger’sCat | Heisenbug | TwentyOne | TwentyTwo | TwentyThree"
UBUNTU_SUPPORTED_VERS="intrepid | jaunty | karmic | lucid | maverick | natty | oneiric | precise | quantal | raring | trusty | utopic | wily | xenial"

COTSON_IMAGES_PATH="/opt/cotson/"
COTSON_DIST_DIR_NAME="dists"
COTSON_IMAGES_DIR_NAME="images"
COTSON_BSDS_DIR_NAME="bsds"
COTSON_STATUS_PATH="log"	#put inside the bootstrap.status file

OUT_FILE="$COTSON_IMAGES_PATH/$COTSON_STATUS_PATH/bootstrap.status"
### ############################ ###

### FUNCTION ###

function sv_exit()
{
	kill -s TERM $TOP_PID
}


function echo_d()
{
	if [ $VERBOSE = 1 ]; then
		verbose_str="[`date +"%T"` - ${FUNCNAME[1]}:${BASH_LINENO[$FUNCNAME[1]]} - D ]: "
	else
		verbose_str="* "
	fi
	if [ $DEBUG = 1 ]; then
		echo -e "\e[33m$verbose_str$1\e[39m"
	else
		return
	fi
}

function echo_w()
{
	if [ $VERBOSE = 1 ]; then
		verbose_str="[`date +"%T"` - ${FUNCNAME[1]}:${BASH_LINENO[$FUNCNAME[1]]} - W ]: "
	else
		verbose_str="* "
	fi
	echo -e "\e[33m$verbose_str$1\e[39m"
}

function echo_e()
{
	if [ $VERBOSE = 1 ]; then
		verbose_str="[`date +"%T"` - ${FUNCNAME[1]}:${BASH_LINENO[$FUNCNAME[1]]} - E ]: "
	else
		verbose_str="* "
	fi
	echo -e "\e[31m$verbose_str$1\e[39m"
}

function echo_ok()
{
	if [ $VERBOSE = 1 ]; then
		verbose_str="[`date +"%T"` - OK]: "
	else
		verbose_str="* "
	fi
	echo -e "\e[32m$verbose_str$1\e[39m"
}

function echo_i()
{
	local verbose_str=""
	if [ $VERBOSE = 1 ]; then
		verbose_str="[`date +"%T"` - I ]: "
	else
		verbose_str="* "
	fi
	echo -e "$verbose_str$1"
}

# FUNCTION: sysctl_tuning()
# Configure the system parameters with the correct values for running COTSon
# ROOT required!
# Tune /etc/sysctl.conf values
#
function sysctl_tuning()
{
	#
	# sysctl -w vm.max_map_count=65535
	# REFERENCE: https://www.kernel.org/doc/Documentation/sysctl/vm.txt
	# max_map_count:
	# This file contains the maximum number of memory map areas a process may 
	# have. Memory map areas are used as a side-effect of calling malloc, 
	# directly by mmap and mprotect, and also when loading shared libraries.
	# While most applications need less than a thousand maps, certain programs, 
	# particularly malloc debuggers, may consume lots of them, e.g., up to one or 
	# two maps per allocation.
	# The default value is 65536.
	#
	echo_i "Tuning /etc/sysctl.conf..."
	mapcurrent=`/sbin/sysctl -n vm.max_map_count`
	# calculate an optimal value as totmem/4 (min 4M)
	total_ram_mem=`free |awk '/Mem/{printf("%d", ($2+1048575)/1048576)}'` #round to GB
	echo_i "  Total RAM Memory system     = about $total_ram_mem GB"
	mapoptim=`expr $total_ram_mem \/ 4`
	mapmin=4194304
	echo_i "  mapoptim   = $mapoptim"
	echo_i "  mapmin     = $mapmin"
	
	if [ $mapoptim -lt 4 ]; then 
		mapok=$mapmin; 
	else 
		mapok=`expr $mapoptim \* 1048576`; 
	fi
	echo_i "  mapok      = $mapok"
	
	mapconfig=`egrep "[ \t]*[^#]*[ \t]*vm.max_map_count" /etc/sysctl.conf| tail -1 |awk -F= '{print $2}'`
	if [ "$mapconfig" = "" ]; then mapconfig="0"; fi
	echo_i "  mapconfig  = $mapconfig"
	echo_i "  mapcurrent = $mapcurrent"
	#Tune vm.max_map_count
	if [ $mapcurrent -lt $mapok ]; then
		echo_i "Tuning vm.max_map_count : $mapcurrent --> $mapok"
		#case 1: the sysctl.conf contains a vm.max_map_count line
		sed -i "s/^[ \t]*[^#]*[ \t]*vm.max_map_count[[:space:]]*=.*/vm.max_map_count=$mapok/" /etc/sysctl.conf
		#case 2: the sysctl.conf DOES NOT contaiin a vm.max_map_count
		if [ "$mapconfig" = 0 ]; then 
			sh -c "echo vm.max_map_count=$mapok >> /etc/sysctl.conf"
		fi
		sysctl -p 2>&1 >/dev/null # make it permanent
		# verify settings
		echo_i "Verifying settings"
		mapconfig2=`egrep "[ \t]*[^#]*[ \t]*vm.max_map_count" /etc/sysctl.conf| tail -1 |awk -F= '{print $2}'`
		mapcurrent2=`/sbin/sysctl -n vm.max_map_count` # this may not require sudo access
		echo_i "  mapconfig2 = $mapconfig2"
		echo_i "  mapcurrent2= $mapcurrent2"
		echo_i "* New VM configuration is: vm.max_map_count = $mapcurrent2"
	else
		echo_i "* Detected that VM configuration is ok: vm.max_map_count = $mapcurrent"
	fi
	echo "sysctl_tuning=\"OK\"" >> $OUT_FILE
}

function _detect_os()
{
	echo_d "CALL: $FUNCNAME"
	ret=`which lsb_release 2>&1`
	check_status1=`echo "$ret"|grep "not found"`
	check_status2=`echo "$ret"|grep "no lsb_release"`
	ec1=0
	ec2=0
	if [[ "$check_status1" != "" || "$check_status2" != "" ]]; then
		#don't know yet which distro
		#try
		echo_w "lsb_release not found... try to install"
		yum -y install redhat-lsb 2>/dev/null
		ec1=$?
		#try
		apt-get -q -q install lsb 2>/dev/null
		ec2=$?
	fi

	if [[ $ec1 != 0 && $ec2 != 0 ]]; then
		echo_e "Couldn't find 'lsb_release'command"
		echo_e "Plese check your environment"
		echo "BOOTSTRAP=\"KO\"" > $OUT_FILE
		exit 1
	fi

	local dist
	dist=`lsb_release -i -s`
	DIST=$dist
	dist1="$dist"
	DIST1=$dist1
	ver=`lsb_release -c -s`
	VER=$ver
	vernum=`lsb_release -r -s`
	VERNUM=$vernum
	echo "DIST=$DIST" >> $OUT_FILE
	echo "DIST1=$DIST1" >> $OUT_FILE
	echo "VER=$VER" >> $OUT_FILE
	echo "VERNUM=$VERNUM" >> $OUT_FILE

	#compatibility patch for CentOS
	if [ "$DIST" = "CentOS" ]; then
	   dist="Fedora"
	   dvn1=${vernum%.*}
	   if [ "$dvn1" = "6" ]; then vernum="14"; ver="Laughlin"; fi
	fi

	echo_i "Detected Distribution '$dist' - Version '$ver'"
	ok=0
	shopt -s extglob
	local ubuntu_check='@('${UBUNTU_SUPPORTED_VERS// /}')'
	local debian_check='@('${DEBIAN_SUPPORTED_VERS// /}')'
	local fedora_check='@('${FEDORA_SUPPORTED_VERS// /}')'
	case $dist in
		Ubuntu)
			case "$ver" in
				$ubuntu_check) ok=1 ;;
			esac
		;;
		Debian)
			case "$ver" in
				$debian_check) ok=1 ;;
			esac
		;;
		Fedora)
			case "$ver" in
				$fedora_check) ok=1 ;;
			esac
		;;
	esac
	if [[ $ok -eq 0 ]]; then
		echo ""
		echo "Sorry: $dist $ver not supported"
		echo "Supported versions are:"
		echo "   UBUNTU: $UBUNTU_SUPPORTED_VERS"
		echo "   DEBIAN: $DEBIAN_SUPPORTED_VERS"
		echo "   FEDORA: $FEDORA_SUPPORTED_VERS"
		echo ""
		echo "BOOTSTRAP=\"KO\"" > $OUT_FILE
		exit 1;
	fi

	echo_i "$dist $ver supported"
	echo "_detect_os=\"OK\"" >> $OUT_FILE
}

function make_dependencies()
{
	_detect_os
	if [[ $DIST == "Debian" ||  $DIST == "Ubuntu" ]]; then
		pqxx=libpqxx-dev 
		ruby="ruby ruby1.8 rubygems libopenssl-ruby libsqlite3-ruby "

		if [[ "$VER" == "wily" || "$VER" == "xenial" ]]; then
			check_repo=`apt-add-repository universe`
			ret=`echo "$check_repo"|grep "distribution component is already enabled"`
			echo_d "UNIVERSE: $ret"
			if [ "$ret" = "" ]; then
				apt-get -y update
			fi
			ruby="ruby"
		fi
		if [[ $VER == "precise" || $VER == "quantal" || $VER == "raring" ]]; then 
			pqxx=libpqxx3-dev
		fi
		if [[ $VER == "trusty" || $VER == "utopic" || $VER == "jessie" || $VER == "wheezy" ]]; then
			pqxx=libpqxx3-dev
			if [ "$VER" = "utopic" ]; then ruby=""; else ruby="ruby1.8"; fi
			if [ "$VER" = "trusty" ]; then ruby="ruby"; fi
		fi
		pkgs=$DEBIAN_BASED_PKGS
		pkgs+="$pqxx "
		pkgs+="$ruby "
		echo_d "$pkgs"
		#pkgs="g++ g++-multilib subversion genisoimage bison flex vnc4server  \
		#rxvt xfwm4 xfonts-100dpi xfonts-75dpi zsh sharutils build-essential xvnc4viewer screen \
		#liblua5.1-0 liblua5.1-0-dev $ruby $pqxx zlib1g-dev indent xutils-dev libsqlite3-dev \
		#sqlite3 libdbd-sqlite3-perl libdbd-pg-perl gnuplot libboost-dev libboost-thread-dev lzma \
		#libxcursor1 libxrender1 libsm6 libxi6 libfontconfig1 "
		
		cmdi="apt-get -q -q"
		cmdt1=""
		cmdt2="dpkg -s"
		cmdcheckstr="^Status.*installed$"
		xvnc="Xvnc4"
		xvncservctrl="vnc4server"
		vncargs1="-ac -SecurityTypes None -br -pn -httpd /usr/share/"
		vncargs2="-desktop="
		llua="lua5.1"

		if [ "$VER" != "utopic" -a "$VER" != "wily" -a "$VER" != "trusty" ]; then
			ruby --version | grep 1.8
			if [[ $? -ne 0 ]]; then
				echo "### Installing ruby1.8"
				apt-get install ruby1.8 -y
				ruby18=`update-alternatives --list ruby | grep 1.8`
				update-alternatives --set ruby $ruby18
			fi
		fi
	elif [[ $DIST == "Fedora" ]]; then
		if [[ $VERNUM -lt 17 ]]; then
			rubystuff="ruby-sqlite3 rubygem-test-unit"
		else
			rubystuff="rubygem-sqlite3 rubygem-bigdecimal rubygem-test-unit"
		fi
		if [ "$DIST1" = "Fedora" ]; then # avoid the following pkgs in CentOs
			extrapkgs="dpkg xfwm4 perl-pgsql_perl5 libstdc++-static"
		fi
		if [[ $VERNUM -ge 20 ]]; then
			extrapkgs="$extrapkgs compat-lua* mariadb"
		fi
		pkgs=$FEDORA_BASED_PKGS
		pkgs+="${rubystuff} "
		pkgs+="$extrapkgs "

		if [[ `uname -m` == "x86_64" ]]; then 
			screen_downgrade="ftp://ftp.pbone.net/mirror/archive.fedoraproject.org/fedora/linux/releases/13/Fedora/x86_64/os/Packages/screen-4.0.3-15.fc12.x86_64.rpm"
		else
			screen_downgrade="ftp://ftp.pbone.net/mirror/archive.fedoraproject.org/fedora/linux/releases/13/Fedora/i386/os/Packages/screen-4.0.3-15.fc12.i686.rpm"
		fi
		cmdi="yum -y"
		#cmdt1="yum list install"
		cmdt1="yum list installed"
		cmdt2=""
		cmdcheckstr=""
		xvnc="Xvnc"
		xvncservctrl="vncserver"
		vncargs1="-ac -SecurityTypes None -br -pn -httpd /usr/share/"
		vncargs2="-desktop "
		if [[ $VERNUM -ge 20 ]]; then
			llua="lua-5.1"
		else
			llua="lua"
		fi

		#SELinux complains if the SimNow LinuxLibs execute code in the stack
		#The following command clears the stack-execute bit in the SimNow LinuxLibs
		#(alternatively, SELinux can be disabled)
		execstack -c $g_simnow_dir/linuxlibs/lib*.so.*
	fi
	########################################################################

	echo_i "* Checking if extra packages should be installed ..."
	ret="0"
	echo -n "  "
	[ -z "$cmdt1" ] || pkginst=`$cmdt1`
	for p in $pkgs; do
		#echo -n "$p "
		echo -n ". "
		[ -z "$cmdt2" ] && cmdcheckstr="$p$cmdcheckstr" || pkginst=`$cmdt2 $p 2>/dev/null`
		(echo "$pkginst" |grep -q "$cmdcheckstr") >/dev/null 2>/dev/null
		ec=$?
		if [ "$ec" != "0" ]; then
			ret=`expr $ret + 1`;
			if [ "$ppkgs" = "" ]; then
				ppkgs="$p"; 
			else
				ppkgs="$ppkgs $p";
			fi
		fi
	done
	echo ""
	ret1="0"
	if [ "$ret" != "0" ]; then
		echo ""
		echo "*** NOTE: for this step (installation of missing packages/dependencies: $ppkgs ), either ask your admin or be sure you have sudo access"
		echo "* Installing missing packages/dependencies..."
		echo_d "cmdi= $cmdi - ppkgs= $ppkgs"
		$cmdi install $ppkgs
		ec="$?"
		echo_d "ec=$ec"
		if [ "$ec" != "0" ]; then
			ret1=`expr $ret1 + 1`;
		fi
		echo_i "* Package installation FINISHED"
	else
		echo_i "ALL needed packages are present"
	fi

	if [[ "$ret1" -ne 0 ]]; then
		echo ""
		echo "ERROR: Could not install one or more of $pkgs"
		echo "- Please check your distribution and edit 'configure' to reflect it"
		echo "BOOTSTRAP=\"KO\"" > $OUT_FILE
		exit 1
	fi

	# Workaround for screen 4.1 bug
	if [[ $dist == "Fedora" && `screen -v | awk  '$3 != "4.00.03"'` ]]; then
		echo ""
		echo "Downgrade of package ($screen_downgrade)"
		echo "*** NOTE: sudo access required ***"
		echo ""
		$cmdi downgrade $screen_downgrade
		grep screen /etc/yum.conf
		if [[ $? -ne 0 ]]; then
		        echo "Add screen to exclude entry in yum.conf"	
			sh -c "echo 'exclude=screen' >> /etc/yum.conf" 
		fi
	fi
	# end workaround 
	# Workaround for rubygems-psych bug
	if [[ $dist == "Fedora" && $vernum == 19 ]]; then
		yum update --enablerepo=updates-testing ruby-2.0.0.247-13.fc19
	fi
	# end workaround 
	echo "make_dependencies=\"OK\"" >> $OUT_FILE
}

function cotson_images_dirs()
{
	echo_i "Checking the cotson directory ..."
	if [ ! -e $COTSON_IMAGES_PATH ]; then
		echo_i "Cotson directory structure not exist"
		echo_i "Try to create them..."
		echo_d "mkdir -p $COTSON_IMAGES_PATH"
		mkdir -p $COTSON_IMAGES_PATH
		echo_d "mkdir -p $COTSON_IMAGES_PATH/$COTSON_DIST_DIR_NAME"
		mkdir -p $COTSON_IMAGES_PATH/$COTSON_DIST_DIR_NAME
		chmod o+w $COTSON_IMAGES_PATH/$COTSON_DIST_DIR_NAME
		echo_d "mkdir -p $COTSON_IMAGES_PATH/$COTSON_IMAGES_DIR_NAME"
		mkdir -p $COTSON_IMAGES_PATH/$COTSON_IMAGES_DIR_NAME
		chmod o+w $COTSON_IMAGES_PATH/$COTSON_IMAGES_DIR_NAME
		echo_d "mkdir -p $COTSON_IMAGES_PATH/$COTSON_BSDS_DIR_NAME"
		mkdir -p $COTSON_IMAGES_PATH/$COTSON_BSDS_DIR_NAME
		chmod o+w $COTSON_IMAGES_PATH/$COTSON_BSDS_DIR_NAME
		echo_d "mkdir -p $COTSON_IMAGES_PATH/$COTSON_STATUS_PATH"
		mkdir -p $COTSON_IMAGES_PATH/$COTSON_STATUS_PATH
		chmod o+w $COTSON_IMAGES_PATH/$COTSON_STATUS_PATH
		# need to check ??

	elif [[ -e $COTSON_IMAGES_PATH/$COTSON_DIST_DIR_NAME && 
		-e $COTSON_IMAGES_PATH/$COTSON_IMAGES_DIR_NAME && 
		-e $COTSON_IMAGES_PATH/$COTSON_BSDS_DIR_NAME && 
		-e $COTSON_IMAGES_PATH/$COTSON_STATUS_PATH ]]; then
		echo_i "COTSon directory for images already exist."
	else
		echo_e "ERROR in COTSon directory structure"
		echo_i "try to recreate the directory structure"
		echo_d "mkdir -p $COTSON_IMAGES_PATH/$COTSON_DIST_DIR_NAME"
		mkdir -p $COTSON_IMAGES_PATH/$COTSON_DIST_DIR_NAME
		chmod o+w $COTSON_IMAGES_PATH/$COTSON_DIST_DIR_NAME
		echo_d "mkdir -p $COTSON_IMAGES_PATH/$COTSON_IMAGES_DIR_NAME"
		mkdir -p $COTSON_IMAGES_PATH/$COTSON_IMAGES_DIR_NAME
		chmod o+w $COTSON_IMAGES_PATH/$COTSON_IMAGES_DIR_NAME
		echo_d "mkdir -p $COTSON_IMAGES_PATH/$COTSON_BSDS_DIR_NAME"
		mkdir -p $COTSON_IMAGES_PATH/$COTSON_BSDS_DIR_NAME
		chmod o+w $COTSON_IMAGES_PATH/$COTSON_BSDS_DIR_NAME
		echo_d "mkdir -p $COTSON_IMAGES_PATH/$COTSON_STATUS_PATH"
		mkdir -p $COTSON_IMAGES_PATH/$COTSON_STATUS_PATH
		chmod o+w $COTSON_IMAGES_PATH/$COTSON_STATUS_PATH
		ls -l /opt/cotson/
		exit 1
	fi
}

function usage()
{
	echo -e "
  \n  COTSon bootstrap utiliy\n
  ATTENTION: Require ROOT permitions!
  example: sudo ./bootstrap.sh [OPTIONS]
  OPTIONS:
  \t-h, --help: show this message
  \t    --simnow_dir simnow_location: set simnow dir
  \t-d, --debug: run in debug mode\n
  \t-v, --verbose: run in verbose mode\n"
#  \t-a, --add: add new images\n"
	exit 1
}

function check_simnow()
{
	echo_d "PARMS: $@"
	echo_i "TO BE TEST"
	if [[ "$SIMNOW_DIR" != "" ]]; then
		g_simnow_dir=$SIMNOW_DIR
	fi
	if [[ "$g_simnow_dir" == "" ]]; then
		bdir1="${root}"
		bdir2=`dirname $bdir1`
		bdir3=`dirname $bdir2`
		bdir4=`dirname $bdir3`
		sdir="simnow-linux64-${SIMNOW_SUPPORTED_VER}"
		# Make some attempt to automatically locate a simnow installation
		if [[ -d "$bdir4/$sdir" ]]; then g_simnow_dir="$bdir4/$sdir"; fi
		if [[ -d "$bdir3/$sdir" ]]; then g_simnow_dir="$bdir3/$sdir"; fi

		if [[ -d "$bdir2/$sdir" ]]; then g_simnow_dir="$bdir2/$sdir"; fi
		if [[ -d "$bdir1/$sdir" ]]; then g_simnow_dir="$bdir1/$sdir"; fi
	fi

	if [[ "$g_simnow_dir" == "" ]]; then
		echo ""
		echo "ERROR: Cannot find SimNow"
		echo "       You have to define the SIMNOW_DIR env variable"
		echo "       or pass '--simnow_dir <simnow_directory>' to $0"
		echo ""
		echo "       To download a free copy of simnow, please visit"
		echo "       http://developer.amd.com/cpu/simnow"
		echo ""
		exit 1
	fi

	if [[ ! -d "$g_simnow_dir" ]]; then
		echo_e "ERROR: Directory '$g_simnow_dir' not found..."
		exit 1
	fi

	#info
	echo_i "* Detected SIMNOW_DIR: '$g_simnow_dir'"
	return 0
}

######################################################################## MAIN ##

case $1 in
		--simnow_dir) g_simnow_dir=$2; shift; shift;;
		-d|--debug) DEBUG=1; shift; shift;;
		-v|--verbose) VERBOSE=1; shift; shift;;
		--help|-h) usage;;
esac

if [ "$(id -u)" != "0" ]; then
	echo_e "This script must be run as root" 1>&2
	exit 1
fi

cotson_images_dirs
echo "# bootatrap.sh status file" > $OUT_FILE
check_simnow "NULL"
sysctl_tuning
make_dependencies
echo "g_simnow_dir=$g_simnow_dir" >> $OUT_FILE
echo "BOOTSTRAP=\"OK\"" >> $OUT_FILE

########################################################################
# Fix missing rgb.txt if needed
if [[ ! -f /etc/X11/rgb.txt ]]; then
	cp etc/rgb.txt /etc/X11/rgb.txt
fi
echo_i "System is ready for COTSon installation."
echo_i "Download default image: $DIST_IMAGE"
#./get-images.sh -d $DIST_IMAGE
./add-image.sh $DIST_IMAGE
