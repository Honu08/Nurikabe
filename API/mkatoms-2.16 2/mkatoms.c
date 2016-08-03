/*
**  mkatoms 2.16 - split a one-line list of atoms into one line per atom.
**
** (C) Copyright Marcello Balduccini 2002-2016  All Rights Reserved
** 
** marcello.balduccini@gmail.com
**
**
**   History
**  ~~~~~~~~~
**   06/28/16 - [2.16]map -f to neg(f) when using -m -a.
**   01/04/16 - [2.15]fixed handling of escaped double quotes.
**   06/20/13 - [2.14]fixed problem with dlv and multiple models.
**   01/16/13 - [2.13]added support for optimal answer sets output
**                    by clingof.
**   06/16/11 - [2.12]mkatoms can now be cross-compiled for Windows
**                    under mingw32.
**   03/23/11 - [2.11]added support for optimal answer sets output
**                    by clasp.
**   06/10/10 - [2.10]fixed bug that causes mkatoms to crash under
**                    MacOS.
**   05/04/09 - [2.9] command line option "-n" added.
**   07/30/08 - [2.8] ability to output a single "meta-model" added.
**                    The meta-model is encoded by atoms of the form
**                                 holds_in(l,k)
**                    where l is a literal from the (actual) model
**                    and k is the 1-based index of the model.
**   07/30/08 - [2.7] support added for clasp.
**   06/21/05 - [2.6] support added for constants in double quotes.
**   01/02/05 - [2.5] end-of-atom detection corrected when parsing
**		      dlv output (could create problems with large
**		      files).
**   03/09/04 - [2.4] auto-detection of cr/cr-lf disabled if table
**		      generation is not requested.
**   05/12/03 - [2.3] input checking made more robust.
**   04/15/02 - [2.2] added ability to handle cmodels output. The type
**   		      of the file (smodels/dlv/cmodels) is *still*
**   		      auto-detected.
**   08/19/02 - [2.1] fixed bug with dlv detection (thanks Greg)
**   08/13/02 - [2.0] added ability to handle dlv output. The type of
**   		      the file (smodels/dlv) is auto-detected.
**   08/07/02 - [1.9] fixed bug in the handling of models longer than
** 		      clen chars.
**		      Added -a option.
**   02/06/02 - [1.8] introduced auto-configuration using "configure.in"
**   02/06/02 - [1.7] use of mkstemp() made compatible with BSD 4.3
**   08/29/01 - [1.6] First public release
**   05/18/00 - removed ftell() so mkatoms works with pipes.
**		Auto-detection of cr/cr-lf
**   06/11/99 - First version
**
*/
#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define VERSION "2.16"

#define clen 102400

#define MOD_NONE	0
#define MOD_SMODELS	1
#define MOD_DLV		2
#define MOD_CMODELS	3
#define MOD_CLASP	4

void print_usage(void)
{	fprintf(stderr,"MKAtoms v "VERSION"\n");
	fprintf(stderr,"(C) Copyright Marcello Balduccini (marcello.balduccini@gmail.com)\n");
	fprintf(stderr,"    2002-2016  All Rights Reserved\n");
	fprintf(stderr,"\n");
	fprintf(stderr,"Usage: mkatoms [-a] [-m] [-n] [-h] [-t]\n");
	fprintf(stderr,"          -a:          format the output so that it can be used\n");
	fprintf(stderr,"                       as an A-Prolog program.\n");
	fprintf(stderr,"          -m:          encode the output as a single 'meta-model',\n");
	fprintf(stderr,"                       that is each literal l is rewritten as\n");
	fprintf(stderr,"                                   holds_in(l,k).\n");
	fprintf(stderr,"                       where k is the 1-based index of the current model.\n");
//	fprintf(stderr,"                       Notice that the -m option can be used together with -a.\n");
	fprintf(stderr,"          -n:          do not output the ::endmodel or %%endmodel marker.\n");
	fprintf(stderr,"                       Note: not recommended with multiple models.\n");
	fprintf(stderr,"          -h:          print this help.\n");
	fprintf(stderr,"          -t:          undocumented -- internal use only.\n");
}

/* checks if a starts with key */
int startsWith(char *a,char *key)
{	if (strncmp(a,key,strlen(key))==0)
		return(1);
	return(0);
}

int trimNewLines(char *line)
{	int l;
	int new_line;

	new_line=0;
	for(l=strlen(line);
	    l>0 && (line[l-1]=='\n' || line[l-1]=='\r');
	    l--)
	{	line[l-1]=0;
		new_line=1;
	}
	return(new_line);
}

/* check if the line is "Answer: ###" */
int is_answer_line(char *l)
{	return(startsWith(l,"Answer:"));
}

int is_optimization_line(char *l)
{	return(startsWith(l,"Optimization: "));
}

int is_optimum_found_line(char *l)
{	return(startsWith(l,"OPTIMUM FOUND"));
}

int is_smodels_model(char *l)
{	return(startsWith(l,"Stable Model:"));
}

int is_dlv_model(char *l)
{	if (startsWith(l,"Best model:"))
		return(1);

	return(l[0]=='{');
}

int is_cmodels_model(char *l)
{	return(startsWith(l," Answer set:"));
}

int is_model(char *l,int prev_line_was_answer_num,int *type)
{	int new_type=MOD_NONE;

	if (is_smodels_model(l))
		new_type=MOD_SMODELS;
	else if (is_dlv_model(l))
		new_type=MOD_DLV;
	else if (is_cmodels_model(l))
		new_type=MOD_CMODELS;
	else if (prev_line_was_answer_num)
		/* Clasp format:
		 * line 1: Answer: ### 
		 * line 2 has no prefix -- it starts with the model right away.
		 */
		new_type=MOD_CLASP;
	else
		return(0);

	if (*type==MOD_NONE)
	{	*type=new_type;
		return(1);
	}

	return(new_type==*type);
}

/* NON-REENTRANT!!! */
char *make_temp_path(char *fname)
{	static char *buff=NULL;
	char *tmpdir;

	if (buff!=NULL)
		free(buff);

#ifdef __MINGW32__ 
	tmpdir=getenv("TEMP");
	if (tmpdir==NULL)
#endif
	tmpdir="/tmp";
	buff=calloc(strlen(tmpdir)+strlen(fname)+2,sizeof(char));
	sprintf(buff,"%s/%s",tmpdir,fname);

	return(buff);
}

#if !defined(HAVE_MKSTEMP) && defined(HAVE_MKSTEMPS)
extern int mkstemps (char *pattern, int suffix_len); 
#endif

// e.g. fp=create_temp("/tmp/mkatoms.XXXXXX",&name)
FILE *create_temp(const char *templ,char **name)
{       int h;
        FILE *fp;
        char *fname;

        fname=strdup(templ);

#ifdef HAVE_MKSTEMP
        h=mkstemp(fname);
#else
#  ifdef HAVE_MKSTEMPS
        h=mkstemps(fname,0);
#  else
#    error "Either mkstemp() or mkstemps() must be available."
#  endif
#endif
        if (h<0)
        {       printf("Unable to create temp file. With Windows, make sure directory C:\\tmp exists.\n");
                exit(1);
        }

        fp=fdopen(h,"w");
        *name=fname;

        return(fp);
}

void readfile(char *file)
{	char b[clen];
	FILE *fp;

	fp=fopen(file,"r");
	if (fp==NULL)
	{	printf("Unable to read from file %s.\n",file);
		exit(1);
	}

	while(!feof(fp))
	{	int sz;
	
		sz=fread(b,sizeof(char),clen,fp);
		if (sz>0)
			fwrite(b,sizeof(char),sz,stdout);
	}

	fclose(fp);
}

int main (int argc,char *argv[])
{	char *line,*p=NULL,*tline;
	int table=0;
	int meta_model=0;
	int A_prolog_like=0;
	int show_endmodel_marker=1;
	long pos;
	int /*tnl,*/new_line;
	FILE *fp;
	int t,crlf_addon=0;
	int prev_line_was_answer_num;
	int models;
	int type;
	int optimization_output;
	int is_optimum;
	int inside_quotes;
	char *TFILE_name=NULL;
	FILE *TFILE;
	int model_in_TFILE;
	long pos_in_TFILE;

	while(argc>=2)
	{	if (strcmp(argv[1],"-h")==0)
		{	print_usage();
			exit(0);
		}
		else
		if (strcmp(argv[1],"-t")==0)
		{	table=1;
			argc--; argv++;
		}
		else
		if (strcmp(argv[1],"-m")==0)
		{	meta_model=1;
			argc--; argv++;
		}
		else
		if (strcmp(argv[1],"-n")==0)
		{	show_endmodel_marker=0;
			argc--; argv++;
		}
		else
		if (strcmp(argv[1],"-a")==0)
		{	A_prolog_like=1;
			argc--; argv++;
		}
		else
		{	printf("Unrecognized option \'%s\'\n",argv[1]);
			print_usage();
			exit(1);
		}
	}

	line=calloc(clen+1,sizeof(char));

	if (table)
	{	//sprintf(line,"/tmp/tmp.XXXXXX");
		strcpy(line,make_temp_path("tmp.XXXXXX"));
#ifdef HAVE_MKSTEMP
		t=mkstemp(line);
#else
#  ifdef HAVE_MKSTEMPS
		t=mkstemps(line,0);
#  else
#    error "Either mkstemp() or mkstemps() must be available."
#  endif
#endif
		if (t<0)
		{	printf("Unable to create temp file. With Windows, make sure directory C:\\tmp exists.\n");
			exit(1);
		}
		fp=fdopen(t,"wt");
		if (fp==NULL)
		{	printf("Unable to create temp file. With Windows, make sure directory C:\\tmp exists.\n");
			exit(1);
		}
		fprintf(fp,"\n");
		crlf_addon=ftell(fp)-1;
		fclose(fp);
		unlink(line);
	}

	line[0]=0;

	pos=0;
	new_line=1;
	prev_line_was_answer_num=0;
	models=0;
	type=MOD_NONE;
	optimization_output=0;
	is_optimum=0;
	model_in_TFILE=0;
	pos_in_TFILE=0;
	while (!feof(stdin))
	{	if (fgets(line,clen,stdin)==NULL)
			break;

/*
 * ASSUMPTION
 *
 * Every line that does not contain a model is shorter than clen.
 */

		//tnl=new_line;
		new_line=0;
		new_line=trimNewLines(line);

		/* If we still have a previous model, now we know if
		 * it was a regular model and not a suboptimal solution
		 * (when using clasp and #minimize/#maximize).
		 * If so, output it.
		 */
		if (model_in_TFILE &&
		    (type!=MOD_CLASP || is_answer_line(line)) &&
		    (!optimization_output || is_optimum))
		{	models++;
			if (table)
				fprintf(stderr,"%ld\n",pos_in_TFILE);
			readfile(TFILE_name);
			model_in_TFILE=0;
		}

		if (/*(tnl) && */(is_answer_line(line)))
		{	prev_line_was_answer_num=1;
			continue;
		}
		else
		if (type==MOD_CLASP && model_in_TFILE && is_optimization_line(line))
		{	optimization_output=1;
			continue;
		}
		else
		if (type==MOD_CLASP && model_in_TFILE && is_optimum_found_line(line))
		{	is_optimum=1;
			continue;
		}

		if (/*(tnl) && */(is_model(line,prev_line_was_answer_num,&type)))
		{	int read_more,trm,i=0;
			char *l;

			tline=line;

			switch(type)
			{	case MOD_SMODELS:
				case MOD_CMODELS:
					while (*tline!=':') tline++;
					tline+=2; 	/* skip leading space */
					break;
				case MOD_DLV:
					while (*tline!='{') tline++;
					tline++;	/* skip leading '{' */
					break;
				case MOD_CLASP:
					/* nothing to be skipped on model line */
					break;
			}

			pos_in_TFILE=pos;

			if (TFILE_name==NULL)
//				TFILE=create_temp("/tmp/mkatoms.XXXXXX",&TFILE_name);
				TFILE=create_temp(make_temp_path("mkatoms.XXXXXX"),&TFILE_name);
			else
			{	TFILE=fopen(TFILE_name,"w");
				if (TFILE==NULL)
				{	printf("Unable to create temp file. With Windows, make sure directory C:\\tmp exists.\n");
					exit(1);
				}
			}
			model_in_TFILE=1;
			is_optimum=0;

			read_more=!new_line;
			do
			{	l=tline;

				inside_quotes=0;

				/* check if there are more atoms to read -- see comment below about '}' */
				while ((*l) && (*l!='}'))
				{	p=l;

					/* Find the end of the current atom. This is accomplished
					 * by looking for a delimiter.
					 * ' ' is ok for both smodels and dlv, since the delimiter
					 * for dlv is ', ', and the delimiter for smodels is ' '.
					 * 
					 * Also, we can check for '}' independently of the engine,
					 * since '}' never occurs in the output of smodels.
					 *
					 * Additionally, we have to keep track of escaped quotes (\"),
					 * as those do not count as string delimiters.
					 */
//					while ((*l!=' ') && (*l) && (*l!='}')) l++;
					int escaped=0;
					while ((*l) && (inside_quotes || ((*l!='}') && (*l!=' '))))
					{	if (!escaped && *l=='\"') inside_quotes=!inside_quotes;
						escaped=(*l=='\\');
						l++;
					}

					/* if parsing dlv, remove the comma before the space*/
					if ((type==MOD_DLV) && (*l==' ') && (*(l-1)==','))
						*(l-1)='x';

					if (((*l)==0) && (read_more))		/* if the line did not fit in the buffer... */
					{	for(i=0;i<(int)(l-p);i++)	/* ...move the rest of the line to the beginning */
							line[i]=p[i];		/* of the buffer... */
						line[i]=0;
						l=&line[i];			/* ...and make sure we exit the while loop */
					}
					else
					{	
						*l=0;				/* otherwise, output the current atom */
						if ((type==MOD_DLV) && (*(l-1)=='x'))	/* if parsing dlv, make sure the space that replaced the comma is removed. */
							*(l-1)=0;
						l++;
//						pos+=fprintf(TFILE,"%s%s",p,(A_prolog_like) ? ".\n":"\n")+crlf_addon;
						if (meta_model)
						{	pos+=fprintf(TFILE,"holds_in(");

							if (A_prolog_like && p[0]=='-')
							{	/* negated literal: must translate to neg(f) to make it a legal term*/
								pos+=fprintf(TFILE,"neg(%s)",&p[1]);
							}
							else
								pos+=fprintf(TFILE,"%s",p);
						}
						else
							pos+=fprintf(TFILE,"%s",p);
						if (meta_model) pos+=fprintf(TFILE,",%d)",models);
						/* terminate the line */
						if (A_prolog_like) pos+=fprintf(TFILE,".");
						pos+=fprintf(TFILE,"\n")+crlf_addon;
						i=0;
					}
				}
				trm=read_more;
				if (read_more)	/* if the line did not fit in the buffer, we have to read again */
				{	tline=line;
					if (fgets(&line[i],clen-i,stdin)==NULL)
					{	if (i>0)	/* if the file is over and we still have an atom to write, write it */
							pos+=fprintf(TFILE,"%s%s",p,(A_prolog_like) ? ".\n":"\n")+crlf_addon;
						break;
					}

					read_more=!trimNewLines(line);
					//read_more=1;
					//while ((line[strlen(line)-1]=='\n') ||
					//       (line[strlen(line)-1]=='\r'))
					//{	line[strlen(line)-1]=0;
					//	read_more=0;
					//}
				}
			} while(trm);

			if (show_endmodel_marker && (!meta_model || A_prolog_like))
				pos+=fprintf(TFILE,"%sendmodel\n",(A_prolog_like) ? "%%":"::")+crlf_addon;

			fclose(TFILE);

		}

		prev_line_was_answer_num=0;
	}

	if (model_in_TFILE)
	/* -- the following test is not needed because the latest model is the optimal one in clasp with #minimize and #maximize */
	/* && (!optimization_output || is_optimum) */
	{	models++;
		if (table)
			fprintf(stderr,"%ld\n",pos_in_TFILE);
		readfile(TFILE_name);
	}
	
	if (models==0)
		printf("%s no models found.\n",(A_prolog_like) ? "%***":"***");

	if (TFILE_name!=NULL)
		unlink(TFILE_name);

	exit(0);
}
