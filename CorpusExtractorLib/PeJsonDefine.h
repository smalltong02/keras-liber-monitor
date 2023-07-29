#pragma once

#define PJ_SHA256 "sha256"

#define BI_BASEINFO "baseinfo"
#define BI_FILENAME "filename"
#define BI_FILEPATH "filepath"
#define BI_FILETYPE "filetype"
#define BI_EXE  "exe"
#define BI_FILESIZE "filesize"
#define BI_FILEATTR "fileattributes"
#define BI_ARCHIVE "archive"
#define BI_COMPRESSED "compressed"
#define BI_ENCRYPTED "encrypted"
#define BI_HIDDEN "hidden"
#define BI_NORMAL "normal"
#define BI_READONLY "readonly"
#define BI_SYSTEM "system"
#define BI_TEMPORARY "temporary"

#define VI_VERINFO "versioninfo"
#define VI_DESCRIPTION "description"
#define VI_FILETYPE "filetype"
#define VI_FILEVER "fileversion"
#define VI_PRODUCTNAME "productname"
#define VI_PRODUCTVER "productversion"
#define VI_COPYRIGHT "copyright"
#define VI_VFT_APP "application"
#define VI_VFT_DLL "dll"
#define VI_VFT_DRV "driver"
#define VI_VFT_FONT "font"
#define VI_VFT_STATIC_LIB "static_library"
#define VI_VFT_VXD "virtual_device"

#define II_INSTALLINFO "installinfo"
#define II_SOFTNAME "softwarename"
#define II_SOFTVER "softwareversion"
#define II_INSTALLLOC "installlocation"
#define II_PUBLISHER "publisher"
#define II_PRODUCTID "productid"

#define SI_SIGNINFO "signinfo"
#define SI_TRUE "true"
#define SI_FALSE "false"
#define SI_BSIGNED "bsigned"
#define SI_BVERIFIED "bverified"
#define SI_BCOUNTERSIGN "bcountersign"
#define SI_BMUTILCERTS "bmutilcerts"
#define SI_PRIMARYCERT "primarycert"
#define SI_DIGEST "digest"
#define SI_ISSUER "issuer"
#define SI_SUBJECT "subject"
#define SI_THUMBPRINT "thumbprint"

#define NI_INSIDEINFO "insideinfo"
#define PI_PEINFO "peinfo"
#define PI_EXETYPE "exe_type"
#define PI_IMPORTS "has_imports"
#define PI_EXPORTS "has_exports"
#define PI_RESOURCES "has_resources"
#define PI_SECURITY "has_security"
#define PI_RELOC "has_reloc"
#define PI_TLS "has_tls"
#define PI_BOUND_IMPORT "has_bound_import"
#define PI_DELAY_IMPORT "has_delay_import"
#define PI_DOTNET "is_dotnet"
#define PI_DEBUG "has_debug"
#define PI_SUBSYSTEM "get_subsystem"
#define PI_MINOR_OS_VERSION "get_minor_os_version"
#define PI_MAJOR_OS_VERSION "get_major_os_version"
#define PI_MINOR_SUBSYSTEM_VERSION "get_minor_subsystem_version"
#define PI_MAJOR_SUBSYSTEM_VERSION "get_major_subsystem_version"
#define PI_PEHEADER_START "get_pe_header_start"
#define PI_FILE_ALIGNMENT "get_file_alignment"
#define PI_SIZE_OF_IMAGE "get_size_of_image"
#define PI_ENTRYPOINT "get_ep"
#define PI_RVAS_AND_SIZES "get_number_of_rvas_and_sizes"
#define PI_CHARACTERISTICS "get_characteristics"
#define PI_DLL_CHARACTERISTICS "get_dll_characteristics"
#define PI_SIZE_OF_HEADERS "get_size_of_headers"
#define PI_SIZE_OF_OPTIONAL_HEADER "get_size_of_optional_header"
#define PI_PE_SIGNATURE "get_pe_signature"
#define PI_MAGIC "get_magic"
#define PI_IMAGE_BASE "get_image_base"
#define PI_HEAP_SIZE_COMMIT "get_heap_size_commit"
#define PI_HEAP_SIZE_RESERVE "get_heap_size_reserve"
#define PI_STACK_SIZE_COMMIT "get_stack_size_commit"
#define PI_STACK_SIZE_RESERVE "get_stack_size_reserve"
#define PI_CHECKSUM "get_checksum"
#define PI_VERIFY_CHECKSUM "verify_checksum"
#define PI_TIME_DATE_STAMP "get_time_date_stamp"
#define PI_MACHINE "get_machine"
#define PI_SIZEOF_NT_HEADER "get_sizeof_nt_header"
#define PI_BASE_OF_CODE "get_base_of_code"
#define PI_NUMBER_OF_SECTIONS "get_number_of_sections"
#define PI_SECTION_ALIGNMENT "get_section_alignment"
#define PI_HAS_OVERLAY "has_overlay"
#define PI_ENTROPY "entropy"
#define PI_PACK "pack"
#define PI_MAYPACK "maypack"
#define PI_UNPACK "unpack"

#define TI_TOOLINFO "toolinfo"
#define TI_COMPILER "compiler"
#define TI_LINKER "linker"
#define TI_PACKER "packer"
#define TI_INSTALLER "installer"

#define DD_DATADIRECTORY "datadirectory"
#define DD_EXPORT_DIRECTORY "export_directory"
#define DD_IMPORT_DIRECTORY "import_directory"
#define DD_RESOURCE_DIRECTORY "resource_directory"
#define DD_EXCEPTION_DIRECTORY "exception_directory"
#define DD_SECURITY_DIRECTORY "security_directory"
#define DD_BASERELOC_DIRECTORY "basereloc_directory"
#define DD_DEBUG_DIRECTORY "debug_directory"
#define DD_ARCHITECTURE_DIRECTORY "architecture_directory"
#define DD_GLOBALPTR_DIRECTORY "globalptr_directory"
#define DD_TLS_DIRECTORY "tls_directory"
#define DD_LOADCONFIG_DIRECTORY "loadconfig_directory"
#define DD_BOUNDIMPORT_DIRECTORY "boundimport_directory"
#define DD_IAT_DIRECTORY "iat_directory"
#define DD_DELAYIMPORT_DIRECTORY "delayimport_directory"
#define DD_COMDES_DIRECTORY "comdes_directory"
#define DD_RELADDR "reladdr"
#define DD_SIZE "size"

#define PS_PDBINFOS "pdbinfos"
#define PS_GUID "guid"
#define PS_TYPE "type"
#define PS_TIMESTAMP "timestamp"
#define PS_AGE "age"
#define PS_PATH "path"

#define ED_EMBEDDEDPE "embeddedpe"
#define ED_OFFSET "offset"

#define IT_IMPORTS "imports"
#define IT_NAME "name"
#define IT_FUNCTIONS "functions"

#define ET_EXPORTS "exports"
#define ET_CHARACTERISTICS "characteristics"
#define ET_TIME_DATE_STAMP "Time_Date_Stamp"
#define ET_MAJOR_VERSION "major_version"
#define ET_MINOR_VERSION "minor_version"
#define ET_DLL_NAME "dll_name"
#define ET_BASE "base"
#define ET_NUMBER_OF_FUNCS "number_of_functions"
#define ET_NUMBER_OF_NAMES "number_of_names"
#define ET_ADDR_OF_FUNCS "address_of_functions"
#define ET_ADDR_OF_NAMES "address_of_names"
#define ET_ADDR_OF_NAMES_ORD "address_of_names_ordinals"
#define ET_FUNCTIONS "functions"

#define MN_MAINICON "mainicon"

#define MF_MANIFEST "manifest"
#define MF_ASSEMBLY_IDENTITY "assembly_identity"
#define MF_DESCRIPTION "description"
#define MF_DEPENDENT_ASSEMBLY "dependent_assembly"
#define MF_NAME "name"
#define MF_VERSION "version"
#define MF_TYPE "type"
#define MF_REQUESTED_EXECUTION_LEVEL "requested_execution_level"
#define MF_LEVEL "level"
#define MF_UIACCESS "uiaccess"

#define RH_RICHHEADER "richheader"
#define RH_PRODUCTID "productid"
#define RH_PRODUCTBUILD "productbuild"
#define RH_NUMBEROFUSES "numberofuses"
#define RH_PRODUCTNAME "productname"
#define RH_VSNAME "vsname"

#define SS_SECTIONS "sections"
#define SS_SECNAME "sec_name"
#define SS_INDEX "index"
#define SS_TYPE "type"
#define SS_CHARACTERISTICS "characteristics"
#define SS_RAW_DATA_POINT "raw_data_point"
#define SS_RAW_DATA_SIZE "raw_data_size"
#define SS_VIRTUAL_SIZE "virtual_size"
#define SS_HASDATA "hasdata"
#define SS_HASCODE "hascode"
#define SS_DATAONLY "dataonly"
#define SS_READONLY "readonly"
#define SS_BBS "bbs"
#define SS_DEBUG "debug"
#define SS_INFO "info"
#define SS_ENTROPY "entropy"

#define SF_STRINGS_FEATURE "strings_feature"
#define SF_SECNAME "sec_name"
#define SF_ENTROPY "entropy"
#define SF_INSIDESTR "insidestr"
