/*
 * figure out which are the newest files in a subtree
 * copy them to a destination subtree, deleting enough
 * of the oldest ones in the destination to keep it
 * from growing beyond some defined limit
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <ctype.h>

#include <popt.h>

static struct {
    const char *myName;
    int   haveColumnHeadings;
    char *prefix;
    char *suffix;
    struct {
        char *key;
        int  isComment;
    } column[256];
} g;

#define log( format, ... ) fprintf(stderr, "%s: %s[%d] " format "\n", g.myName, __func__, __LINE__, ##__VA_ARGS__ )

/* Set up a table of options. */
static struct poptOption commandLineOptions[] =
{
    { "prefix",  'p', POPT_ARG_STRING, &g.prefix, 0, "Prefix line with STRING. Defaults to '<partition'", NULL },
    { "suffix",  's', POPT_ARG_STRING, &g.suffix, 0, "Add STRING as suffix to line. Defaults to '/>'", NULL },
    POPT_AUTOALIAS
    POPT_AUTOHELP
    POPT_TABLEEND
};

const char * getLastPathElement( const char *str )
{
    const char *result, *p;

    result = str;
    for ( p = str; *p != '\0'; ++p )
    {
        if (*p == '/')
        {
            result = &p[1];
        }
    }

    return result;
}

char * skipWhitespace( char * string )
{
    char * p = string;
    while ( isspace( *p ) )
    {
        ++p;
    }
    return (p);
}

char *skipPunct( char * string )
{
    char * p = string;
    while ( ispunct( *p ) || isspace( *p ) )
    {
        ++p;
    }
    return (p);
}


char * trimWhitespace( char * string )
{
    char * lastNonSpace = NULL;
    char * p = string;

    while ( *p != '\0' )
    {
        if ( !isspace(*p) )
        {
            lastNonSpace = p;
        }
        ++p;
    }

    if ( lastNonSpace != NULL )
    {
        lastNonSpace[1] = '\0';
    }
    return ( string );
}

int parseLine( const char * line )
{
    char *f, *p, *q;
    char *sep, *fmt;
    int i, len;
    char comment[4096];
    char xml[4096];

    comment[0] = '\0';
    xml[0] = '\0';

    f = strdup( line );
    if ( f != NULL )
    {
        p = f;
        trimWhitespace( p );

        i = 0;
        do {
            sep = strchr( p, ',' );

            if ( sep != NULL )
            {
                /* terminate field (except for the last one, which is already terminated) */
                *sep = '\0';
            }

            if ( g.haveColumnHeadings )
            {
                if ( *p != '\0' )
                {
                    if ( g.column[i].isComment )
                    {
                        q = comment;
                        fmt = "  %s: %s ";
                    }
                    else
                    {
                        q = xml;
                        fmt = " %s=\"%s\"";
                    }

                    len = strlen( q );
                    snprintf( &q[len], sizeof(xml) - len - 1, fmt, g.column[i].key, p );
                }
            }
            else
            {
                if (ispunct( *p ))
                {
                    g.column[i].isComment = 1;
                    p = skipPunct( p );
                }
                g.column[i].key = strdup( p );
            }

            if (sep != NULL)
            {
                p = &sep[1];
            }

            ++i;
        } while ( sep != NULL );

        if (g.haveColumnHeadings)
        {
            if (strlen(comment) > 0)
            {
                printf( "<!--%s -->\n", comment );
            }
            if (strlen(xml) > 0)
            {
                printf( "%s%s %s\n", g.prefix, xml, g.suffix );
            }
        }

        g.haveColumnHeadings = 1;

        free(f);
    }
}

int processLine ( char * line )
{
    char * p;

    p = skipWhitespace( line );

    if ( *p == '\0' )
    {
        /* blank line (zero length or only whitespace) */
        printf( "\n" );
    }
    else if (ispunct( *p ))
    {
        /* pass through verbatim any line that starts with punctuation (i.e. a comment line) */
        /* nuke the spurious commas */
        while (*p != '\0')
        {
            if (*p == ',')
            {
                *p = ' ';
            }
            ++p;
        }
        p = line;
        /* remove any trailing whitespace, including line endings */
        trimWhitespace( p );
        printf( "<!-- %s -->\n", skipPunct( p ) );
    }
    else
    {
        parseLine( p );
    }

    return 0;
}

int main( int argc, const char * argv[] )
{
    int result = 0;
    int rc;
    poptContext context;
    char nextLine[4096];

    g.prefix = "<partition";
    g.suffix = "/>";
    g.haveColumnHeadings = 0;

    g.myName = getLastPathElement( argv[0] );

    /* log( "invoked as \'%s\'", g.myName ); */

    context = poptGetContext( g.myName, argc, argv, commandLineOptions, 0 );

    do {
        rc = poptGetNextOpt( context );
        switch ( rc )
        {
        case -1:
            result = 0;
            while ( result == 0 && fgets( nextLine, sizeof( nextLine ), stdin ) != NULL )
            {
                result = processLine( nextLine );
            }
            break;

        case 0:
            /* help & usage is automatically handled within the popt library */
            break;

        default:
            fprintf( stderr, "%s: %s",
                     poptBadOption( context, POPT_BADOPTION_NOALIAS ),
                     poptStrerror( rc ));
            break;
        }
    } while ( rc > 0 );

    poptFreeContext( context );

    return result;
}
