

#define DIMENSION  7
int _DEBUG_ = 0;
int MAX_LEVEL=8;
int MIN_LEVEL=4;
int FULL=0;
int HTML=0;
int HEX=0;


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <mysql.h>
#include <math.h>

typedef char BoardType[DIMENSION][DIMENSION];

// Globals
BoardType board;
char *STBD=NULL;
BoardType mat1, mat2;
unsigned char white[2];
int jogador = 0;
unsigned char best[2];

MYSQL* DBcon=NULL;

int Floor(float x) {
  return (int)x;
}

int is_null(int x) {
  return ( x<0 );
}


MYSQL *ConnectDB()
{
  MYSQL *con = mysql_init(NULL);
  if (con == NULL) 
  {
      fprintf(stderr, "%s\n", mysql_error(con));
      //exit(1);
  } else
  if (mysql_real_connect(con, "ave.dee.isep.ipp.pt", "jml", "jml2014", 
          NULL, 0, NULL, 0) == NULL) 
  {
      fprintf(stderr, "%s\n", mysql_error(con));
      mysql_close(con);
      return NULL;
  }  
  if (mysql_select_db(con,"jml")) {
      fprintf(stderr, "%s\n", mysql_error(con));
      mysql_close(con);
      return NULL;
  }  
  return con;
}

void PrintBoard(BoardType board) {
  int x, y;
  for( x=0 ; x<DIMENSION+2 ; x++ )
	printf("-");
  printf("\n");
  for( y=0 ; y<DIMENSION ; y++ ) {
    printf("| ");
    for( x=0 ; x<DIMENSION ; x++ )
	printf("%c",board[y][x]);
    printf("|\n");
  }
  for( x=0 ; x<DIMENSION+2 ; x++ )
	printf("-");
  printf("\n");
}

void PrintMat(BoardType board) {
  int x, y;
  for( x=0 ; x<DIMENSION*2+2 ; x++ )
	printf("-");
  printf("\n");
  for( y=0 ; y<DIMENSION ; y++ ) {
    printf("| ");
    for( x=0 ; x<DIMENSION ; x++ )
	printf("%2d ",board[y][x]);
    printf("|\n");
  }
  for( x=0 ; x<DIMENSION*2+2 ; x++ )
	printf("-");
  printf("\n");
}

void AddMat(BoardType mat1, BoardType mat2, BoardType sum) {
  // sum := mat1 + mat2
  int x, y;
  for( y=0 ; y<DIMENSION ; y++ )
  for( x=0 ; x<DIMENSION ; x++ )
	sum[y][x] = mat1[y][x] + mat2[y][x];
}

void SubMat(BoardType mat1, BoardType mat2, BoardType sum) {
  // sum := mat1 - mat2
  int x, y;
  for( y=0 ; y<DIMENSION ; y++ )
  for( x=0 ; x<DIMENSION ; x++ )
	sum[y][x] = mat1[y][x] - mat2[y][x];
}


char* GetSTBD(BoardType board, unsigned char* white, int player, char *stbd)
{
  sprintf(stbd,"%02x%02x%02x%02x%x",DIMENSION,DIMENSION,white[0],white[1],player);
  int byte = 0;
  int count = 0;
  int x, y;
  char block[9];		// 4 should be enough but trigger warning in sprintf
  for( y=0 ; y<DIMENSION ; y++ )
    for( x=0 ; x<DIMENSION ; x++ ) {
      byte += byte;
      if ( board[y][x]=='b' ) byte += 1;
      count++;
      if ( count==8 ) {
        sprintf(block,"%02x",byte);
	strcat(stbd,block);
	byte = 0;
	count = 0;
      }
    }
  if ( count>0 ) {
    while (count<8) {
      byte += byte;
      count++;
    }
    sprintf(block,"%02x",byte);
    strcat(stbd,block);
  }
  return stbd;
}
/*
function PutSTBD($stbd) {
  sscanf($stbd,"%02x%02x%02x%02x%1x", $DIMENSION, $DIMENSION, 
		$_SESSION['white'][0],$_SESSION['white'][1],$_SESSION['jogador']);
  
}
*/
int GetWinner(unsigned char *white) {
  int vencedor = -1;

  if ( white[0]==0 && white[1]==(DIMENSION-1) )
        vencedor = 0;
  else if ( white[1]==0 && white[0]==(DIMENSION-1) )
        vencedor = 1;
//print_r($white);
//echo "venc: $vencedor .";
  return vencedor;
}

int GetBlocked(BoardType board, unsigned char *white) {
  int verdes = 0;
  int x, y;
  for( y=white[1]-1 ; y<DIMENSION && y<=white[1]+1 ; y++ ) {
    if ( y<0 ) continue;
    for( x=white[0]-1 ; x<DIMENSION && x<=white[0]+1 ; x++ ) {
      if ( x<0 ) continue;
      if ( board[y][x]=='b' ) continue;
      if ( x==white[0] && y==white[1] ) continue;
      verdes++;
    }
  }
  return ( verdes==0 );
}

void CreateDijkstraMat(BoardType mat, unsigned char *b)
{
  // Create a new Dijkstra matrix for a free board.
  // Fills mat with Chebyshev distances from b[] cell.
  int x, y;
  for( y=0 ; y<DIMENSION ; y++ )
    for( x=0 ; x<DIMENSION ; x++ )
	mat[y][x] = 40;
  mat[b[1]][b[0]] = 0;

  int v, i, j;
  for( v=0 ; v<DIMENSION ; v++ ) {
    for( y=0 ; y<DIMENSION ; y++ )
    for( x=0 ; x<DIMENSION ; x++ ) {
      if ( mat[y][x]==v ) {
        for( i=x-1 ; i<=x+1 ; i+=1 ) {
          if ( i<0 ) continue;
          if ( i>=DIMENSION ) break;
          for( j=y-1 ; j<=y+1 ; j+=1 ) {
            if ( j<0 ) continue;
            if ( j>=DIMENSION ) break;
	    if ( x==i && y==j ) continue;
	    if ( mat[j][i]>v+1 ) {
		mat[j][i] = (v+1);
	    }
	  }
	}
      }
    }
  }
}


void SetDijkstraMat(BoardType board, BoardType mat, unsigned char *b, unsigned char *opo)
{
  // Create a new Dijkstra matrix for a specified board.
  // Fills mat with Chebyshev distances from b[] cell.
  // opo[] is the Oponent's victory cell. It cannot be used to reach other cells.
  int x, y;
  for( y=0 ; y<DIMENSION ; y++ )
    for( x=0 ; x<DIMENSION ; x++ )
	mat[y][x] = 40;
  mat[b[1]][b[0]] = 0;

  int v, i, j;
  for( v=0 ; v<36 ; v++ ) {
    int count = 0;
    for( y=0 ; y<DIMENSION ; y++ )
    for( x=0 ; x<DIMENSION ; x++ ) {
      if ( board[y][x]=='b' ) continue;
      if ( mat[y][x]==v ) {
        for( i=x-1 ; i<=x+1 ; i+=1 ) {
          if ( i<0 ) continue;
          if ( i>=DIMENSION ) break;
          for( j=y-1 ; j<=y+1 ; j+=1 ) {
            if ( j<0 ) continue;
            if ( j>=DIMENSION ) break;
	    if ( board[j][i]=='b' && ( i!=b[0] || j!=b[1] ) ) continue;
	    if ( x==i && y==j ) continue;
	    //if ( i==opo[0] && j==opo[1] ) continue;

	    if ( abs(i-opo[0])<=2 && abs(j-opo[1])<=1 ) continue;
	    if ( abs(i-opo[0])<=1 && abs(j-opo[1])<=2 ) continue;

	    if ( mat[j][i]>v+1 ) 
		mat[j][i] = v+1;
	    count++;
	  }
	}
      }
    }
    if ( count==0 ) break;
  }
}



int UpdateDijkstraMat(BoardType board, BoardType mat, unsigned char *b)
{
  // Updates an existent Dijkstra matrix for a specified board.
  // b[] cell becomes blocked.
  // Chebyshev distances in mat are upgraded due to new blocked cell.
  int vori = mat[b[1]][b[0]];
/*
echo "v=$v.\n";
echo "b:"; print_r($b);
echo "\nmat ";
print_r($mat);
*/
  int x, y;
  for( y=0 ; y<DIMENSION ; y++ )
  for( x=0 ; x<DIMENSION ; x++ )
    if ( mat[y][x]==-1 || (mat[y][x]>vori /* && $mat[$y][$x]>=$v+max(abs($x-$b[0]),abs($y-$b[1]))*/) ) 
	mat[y][x] = 40;  // not reacheable
  mat[b[1]][b[0]] = 40;
  board[b[1]][b[0]] = 'b';

  int v;
  for( v=vori ; v<36 ; v++ ) {
    int count = 0;
    int i, j;
    for( y=0 ; y<DIMENSION ; y++ )
    for( x=0 ; x<DIMENSION ; x++ ) {
      if ( board[y][x]=='b' ) continue;
      if ( mat[y][x]==v /* && $v>=max(abs($x-$b[0]),abs($y-$b[1]))*/ ) {
	for( i=x-1 ; i<=x+1 ; i+=1 ) {
    	  if ( i<0 ) continue;
    	  if ( i>=DIMENSION ) break;
    	  for( j=y-1 ; j<=y+1 ; j+=1 ) {
	    if ( j<0 ) continue;
	    if ( j>=DIMENSION ) break;
      	    if ( i==x && j==y ) continue;
	    if ( board[j][i]=='b' ) continue;
//echo "p1: x y $x, $y, ij= $i $j ". $mat[$j][$i] . ":\n";
      	    if ( is_null(mat[j][i]) || mat[j][i]>v+1 ) {
//echo "p2: x y $x, $y, ij= $i $j\n";
		mat[j][i] = v+1;
		count++;
	    }
    	  }
  	}
      }
    }
    if ( count==0 ) break;
  }
  return vori;
}

float LoadBestSTBD(char *stbd, unsigned char* best) {  
  // Try to get Best move from DB
  float bestval = -1000.;
  if (DBcon) {
    char query[200];
    sprintf(query,"SELECT Move, Val, MaxLevel FROM STbest WHERE Board='%s' AND MaxLevel>=%d",
	STBD, MAX_LEVEL);
    if ( mysql_query(DBcon, query) )
    	fprintf(stderr, "%s\n", mysql_error(DBcon));
    else {
      MYSQL_RES *result = mysql_store_result(DBcon);
      if ( result==NULL )	fprintf(stderr, "%s\n", mysql_error(DBcon));
      else {
	MYSQL_ROW row = mysql_fetch_row(result);
	if ( row ) {
	  best[0] = row[0][0]-'a';
	  best[1] = DIMENSION+'0'-row[0][1];
	  bestval = atof(row[1]);
	}
	mysql_free_result(result);
      }
    }
  }
  return bestval;
  //echo "<p>Best move: ". chr($best[0]+ord('a')) . ($DIMENSION-$best[1]) . " : " . ($val) . "</p>";
}

float LoadBest(BoardType board, unsigned char* white, int player, unsigned char* best)
{
  if (DBcon) {
    char stbd[DIMENSION*DIMENSION/4+12];
    return LoadBestSTBD( GetSTBD(board, white, player, stbd), best );
  }
  best[0] = best[1] = 0;
  return -10000.;
}

int TopMax(BoardType b, BoardType mat1, BoardType mat2, unsigned char *white, int player)
{

  int steps1 = mat1[white[1]][white[0]];
  if ( is_null(steps1) ) steps1 = 40;

//  $mat2[0][$DIMENSION-1] = 0;
  //UpdateDijkstraMat($b,$mat2,$white);
  int steps2 = mat2[white[1]][white[0]];
  if ( is_null(steps2) ) steps2 = 40;

//echo "stp1: $steps1 $steps2 ";
//printf("stp1: %d %d\n",steps1,steps2 );

  if ( player )  return steps1-steps2;
  return steps2-steps1;
}

float Valor(BoardType board, BoardType mat1, BoardType mat2, unsigned char *white,
		 int player, int level, float max)
{
  int win = GetWinner(white);
  //unsigned char best[2] = "  ";
  max = -100000.;
  if ( win>=0 ) {
    if (win==player )	return  101.;
    else		return -101.;  
  }
  if ( GetBlocked(board,white) ) 
    return -101.;

  // Eminent Win
  if ( player==0 ) {
	if ( white[0]<=1 && white[1]>=DIMENSION-2 ) {
		if (level==0) {
			best[0] = (unsigned char)0;
			best[1] = (unsigned char)(DIMENSION-1);
		}
		return 99.;
	}
  }
  else {
        if ( white[0]>=DIMENSION-2 && white[1]<=0 ) {
                if (level==0) {
			best[0] = (unsigned char)(DIMENSION-1);
			best[1] = (unsigned char)0;
		}	
                return 99.;
        }
  }

  float sum = 0.;

  if ( level<1 ) {
    unsigned char best_loaded[2];
    sum = LoadBest(board,white,player,best_loaded)*(powf(0.99,(float)level));
    if ( sum>-150. ) {
      if ( level==0 ) {
	best[0] = best_loaded[0];
	best[1] = best_loaded[1];
      }
      return sum;
    }
  } 
	


  sum = (float)TopMax(board,mat1,mat2,white,player);
if ( level==0 && _DEBUG_ ) {
  printf("<p>L %d, P %d, xy %d %d, Sum: %f </p>\n",
	level, player, white[0], white[1], sum);
  //print_r($mat1);
  //print_r($mat2);
}
  if (sum+100<max) return sum;

//  echo "stps $steps1 $steps2 sum:$sum ";

  if ( level>MAX_LEVEL ) 
      return sum;
  if ( level>MIN_LEVEL && sum>20 ) return sum;
//    return TopMax($board,$white,$player);
  //$max = -100000000;
  //$sum = 0;
  //if ($level==0) echo "stps $steps1 $steps2 sum:$sum ";

  BoardType nmat1, nmat2;
  memcpy(nmat1, mat1, DIMENSION*DIMENSION);
  memcpy(nmat2, mat2, DIMENSION*DIMENSION);
  //$nmat1 = $mat1;
  //$nmat2 = $mat2;
  
  //$nmat1[$white[1]][$white[0]] = 40;
  //$nmat2[$white[1]][$white[0]] = 40;

  BoardType nboard;
  memcpy(nboard, board, DIMENSION*DIMENSION);
  //$nboard = $board;

  nboard[white[1]][white[0]] = 'b';
  UpdateDijkstraMat(nboard,nmat1,white);
  UpdateDijkstraMat(nboard,nmat2,white);
  int x, y;
  for( y=white[1]+1-player-player ; y>=white[1]-1 && y<=white[1]+1 ; y-=1-player-player ) {
    if ( y<0 || y>=DIMENSION ) continue;
    for( x=white[0]-1+player+player ; x>=white[0]-1 && x<=white[0]+1 ; x+=1-player-player ) {
      if ( x<0 || x>=DIMENSION) continue;
      if ( board[y][x]=='b' ) continue;
      if ( x==white[0] && y==white[1] ) continue;
/*
      if ( $_SESSION['symmetric'] && $level==0 ) {
	if ( $x==$white[0] && $y==$white[1]+1 ) continue;
        if ( $x==$white[0]+1 && $y==$white[1] ) continue;
        if ( $x==$white[0]+1 && $y==$white[1]+1 ) continue;
      }
*/
      unsigned char nwhite[2] = {x, y};
      float nval = -Valor(nboard,nmat1,nmat2,nwhite,(1-player),(level+1),-max)+1;
      sum += nval;

//if ( $level==0 )
	//echo "#xy $x $y $nval $sum\n";

      if ( FULL && level==0 ) {
		printf("  %c%d: %.2f\n", x+'a', DIMENSION-y, nval);
		fflush(stdout);
      }

      if ( nval>max ) {
	max = nval;
	if ( level==0 ) {
		best[0] = x;
		best[1] = y;
	}
      }
      if ( max>=100 ) return (max*0.98+sum*0.002);
    }
  }
  return (max*0.98+sum*0.002);
}



float GetBest(unsigned char* best7) {  
  float bestval = Valor(board, mat1, mat2, white, jogador, 0, -10000000);
  // Save Best
  if (DBcon) {
    char query[200];
    sprintf(query,"SELECT MaxLevel FROM STbest WHERE Board='%s' AND MaxLevel>%d",
	STBD, MAX_LEVEL);
    if ( mysql_query(DBcon, query) )
    	fprintf(stderr, "%s\n", mysql_error(DBcon));
    else {
      MYSQL_RES *result = mysql_store_result(DBcon);
      if ( result==NULL )	fprintf(stderr, "%s\n", mysql_error(DBcon));
      else {
        if ( mysql_num_rows(result)<1 ) { 
          char query[200];
          sprintf(query,"REPLACE STbest(Board,Move,Val,MaxLevel) VALUE ('%s','%c%d',%f,%d) ",
	    STBD,best[0]+'a',DIMENSION-best[1],bestval,MAX_LEVEL );
          if ( mysql_query(DBcon, query) )
    	    fprintf(stderr, "%s\n", mysql_error(DBcon));
	}
	mysql_free_result(result);
      }
    }
  }
  return bestval;
  //echo "<p>Best move: ". chr($best[0]+ord('a')) . ($DIMENSION-$best[1]) . " : " . ($val) . "</p>";
}

void Init() {
    int x, y;
    for( y=0 ; y<DIMENSION ; y++ )
    for( x=0 ; x<DIMENSION ; x++ )
        board[y][x] = (char)-1;

    // Start Position
    white[0] = Floor(DIMENSION/2.+0.5);
    white[1] = Floor(DIMENSION/2.-1.5);

/*
    $_SESSION['mat1'] = array();
    $_SESSION['mat2'] = array();
    //$_SESSION['mat1'][$DIMENSION-1][0] = 0;
    CreateDijkstraMat($_SESSION['mat1'],array(0,$DIMENSION-1));
    //UpdateDijkstraMat($_SESSION['board'],$_SESSION['mat1'],array(0,$DIMENSION-1));
    //$_SESSION['mat2'][0][$DIMENSION-1] = 0;
    CreateDijkstraMat($_SESSION['mat2'],array($DIMENSION-1,0));

    //UpdateDijkstraMat($_SESSION['board'],$_SESSION['mat2'],array($DIMENSION-1,0));
    
    jogador = 0;
    $_SESSION['symmetric'] = 1;
    $_SESSION['player_0'] = 'Local';
    $_SESSION['player_1'] = 'Local';
    $_SESSION['max_level'] = 5;
    $_SESSION['nplayed'] = 0;
*/
}

void Usage() {
  fprintf(stderr,"Usage: slimetrail [Options] STBD [MaxLevel [MinLevel]]\n");
  fprintf(stderr,"    Options: -f    full\n");
  fprintf(stderr,"             -h    html output\n");
  fprintf(stderr,"             -d    debug\n");
  fprintf(stderr,"             -x    hexadecimal output\n");
}


int main(int argc, char **argv)
{
  //printf("%s\n", argv[0]);
  while ( argc>2 && argv[1][0]=='-' ) {
    switch ( argv[1][1] ) {
      case 'f':
	FULL = 1;
	break;
      case 'h':
	HTML = 1;
	break;
      case 'd':
	_DEBUG_ = 1;
	break;
      case 'x':
	HEX = 1;
	break;
      default:
	fprintf(stderr,"Bad option: %s\n", argv[1]);
	Usage();
	return 1;
    }
    argc--;
    argv++;
  }
  if ( argc<2 ) {
	Usage();
	return 1;
  }

  int dim1, dim2, wx, wy;
  STBD = argv[1];
  int res = sscanf(argv[1],"%2x%2x%2x%2x%1x", &dim1, &dim2, &wx, &wy, &jogador);
  if ( res!=5 ) {
	fprintf(stderr,"Bad STBD header\n");
	return 1;
  }
  //printf("%d,%d,%d,%d,%d, res=%d\n", dim1, dim2, wx, wy, jogador, res);
  if ( dim1!=dim2 ) {
    fprintf(stderr,"Non square boards are not supported.\n");
    return 1;
  }
  //DIMENSION = dim1;
  white[0] = (int)wx;
  white[1] = (int)wy;
  if ( wx>=dim2 || wy>=dim1 ) {
    fprintf(stderr,"Invalid white cell position.\n");
    return 1;
  }

  int x, y;
  char *bptr = argv[1]+9;
  //printf("bptr:%s.\n",bptr);
  if (strlen(bptr)<DIMENSION*DIMENSION*2/8) {
	fprintf(stderr,"Bad STBD body\n");
	return 1;
  }
  int cell8;			// 8 Cells in binary
  sscanf(bptr,"%2x",&cell8);
  bptr += 2;
  int cn = 0;			// Cell counter
  int bn = 0;			// Blocked cell counter
  for( y=0 ; y<DIMENSION ; y++ )
  for( x=0 ; x<DIMENSION ; x++ )  {
	if ( cell8 & 0x80 ) {
		board[y][x] = 'b';
		bn++;
	}
	else			board[y][x] = ' ';
	cell8 *= 2;
	if ( ++cn>=8 ) {
		cn = 0;
		sscanf(bptr,"%2x",&cell8);
		bptr += 2;
	}
  }
  board[wy][wx] = 'w';
  if ( _DEBUG_ ) PrintBoard(board);



  unsigned char vcell1[2] = {0,DIMENSION-1};
  unsigned char vcell2[2] = {DIMENSION-1,0};
  SetDijkstraMat(board, mat1, vcell1, vcell2);
  SetDijkstraMat(board, mat2, vcell2, vcell1);
  
  if ( _DEBUG_ ) {
    PrintMat(mat1);
    PrintMat(mat2);
	/*
	printf ("Sum\n");
	BoardType mat_sum;
	AddMat(mat1,mat2,mat_sum);
	PrintMat(mat_sum);
	printf ("Sub\n");
	SubMat(mat1,mat2,mat_sum);
	PrintMat(mat_sum);
	*/
  }
  int vencedor = GetWinner(white);
  if ( vencedor<0 && GetBlocked(board,white) )
	vencedor = 1-jogador;

  if ( vencedor>=0 ) {
	printf ("O jogador %d Venceu!\n", vencedor+1);
	return 0;
  }

  MAX_LEVEL = 8 + bn/4;
  MIN_LEVEL = MAX_LEVEL / 2;
  if ( argc>=3 ) {
	MAX_LEVEL = atoi(argv[2]);
	MIN_LEVEL = MAX_LEVEL/2;
	if ( argc>=4 ) 
		MIN_LEVEL = atoi(argv[3]);
  }

  DBcon = ConnectDB();

  //unsigned char best[2];
  float val = GetBest(best);
  
  if ( DBcon ) mysql_close(DBcon);

  if ( HEX ) 
	printf("%2x%2x %.3f %d\n", best[0], best[1], val, MAX_LEVEL);
  else
	printf("Best Move: %c%d, %.2f, %d\n", best[0]+'a', DIMENSION-best[1], val, MAX_LEVEL);
  return 0;
}
  




