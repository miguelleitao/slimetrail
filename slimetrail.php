<?php
ini_set("error_reporting","E_ALL");
session_start();
$DIMENSION = 7;

function GetSTBD($board,$white,$player) {
  global $DIMENSION;
  $dimstr = sprintf("%02x",$DIMENSION);
  $wstr = sprintf("%02x%02x%x",$white[0],$white[1],$player);
  $stbd = $dimstr.$dimstr.$wstr;
  $byte = 0;
  $count = 0;
  for( $y=0 ; $y<$DIMENSION ; $y++ )
    for( $x=0 ; $x<$DIMENSION ; $x++ ) {
      $byte += $byte;
      if ( $board[$y][$x]==='b' ) $byte += 1;
      $count++;
      if ( $count==8 ) {
	
	//$stbd .= dechex($byte);
	$stbd .= sprintf("%02X", $byte);

	$byte = 0;
	$count = 0;
      }
    }
  if ( $count>0 ) {
    while ($count<8) {
      $byte += $byte;
      $count++;
    }
    //$stbd .= dechex($byte);
    $stbd .= sprintf("%02X", $byte);
  }
  return $stbd;
}

function PutSTBD($stbd) {
  sscanf($stbd,"%02x%02x%02x%02x%1x", $DIMENSION, $DIMENSION, 
		$_SESSION['white'][0],$_SESSION['white'][1],$_SESSION['jogador']);
  
}

function GetWinner($white) {
  $vencedor = "";
  global $DIMENSION;
  if ( $white[0]==0 && $white[1]==($DIMENSION-1) )
        $vencedor = 0;
  else if ( $white[1]==0 && $white[0]==($DIMENSION-1) )
        $vencedor = 1;
//print_r($white);
//echo "venc: $vencedor .";
  return $vencedor;
}

function GetBlocked($board, $white) {
  global $DIMENSION;
  $verdes = 0;
  for( $y=$white[1]-1 ; $y<$DIMENSION && $y<=$white[1]+1 ; $y++ ) {
    if ( $y<0 ) continue;
    for( $x=$white[0]-1 ; $x<$DIMENSION && $x<=$white[0]+1 ; $x++ ) {
      if ( $x<0 ) continue;
      if ( $board[$y][$x]=='b' ) continue;
      if ( $x==$white[0] && $y==$white[1] ) continue;
      $verdes++;
    }
  }
  return ( $verdes==0 );
}
function CreateDijkstraMat(&$mat,$b)
{
  global $DIMENSION;
  for( $y=0 ; $y<$DIMENSION ; $y++ )
    for( $x=0 ; $x<$DIMENSION ; $x++ )
	$mat[$y][$x] = 40;
  $mat[$b[1]][$b[0]] = 0;

  for( $v=0 ; $v<$DIMENSION ; $v++ ) {
    for( $y=0 ; $y<$DIMENSION ; $y++ )
    for( $x=0 ; $x<$DIMENSION ; $x++ ) {
      if ( $mat[$y][$x]===$v ) {
        for( $i=$x-1 ; $i<=$x+1 ; $i+=1 ) {
          if ( $i<0 ) continue;
          if ( $i>=$DIMENSION ) break;
          for( $j=$y-1 ; $j<=$y+1 ; $j+=1 ) {
            if ( $j<0 ) continue;
            if ( $j>=$DIMENSION ) break;
	    if ( $x==$i && $y==$j ) continue;
	    if ( $mat[$j][$i]>$v+1 ) {
		$mat[$j][$i] = ($v+1);
	    }
	  }
	}
      }
    }
  }
}




function UpdateDijkstraMat(&$board,&$mat,$b)
{
  global $DIMENSION;
  $v = $mat[$b[1]][$b[0]];
/*
echo "v=$v.\n";
echo "b:"; print_r($b);
echo "\nmat ";
print_r($mat);
*/
  for( $y=0 ; $y<$DIMENSION ; $y++ )
  for( $x=0 ; $x<$DIMENSION ; $x++ )
    if ( $mat[$y][$x]==="" || ($mat[$y][$x]>$v /* && $mat[$y][$x]>=$v+max(abs($x-$b[0]),abs($y-$b[1]))*/) ) 
	$mat[$y][$x] = 40;  // not reacheable
  $mat[$b[1]][$b[0]] = 40;
  $board[$b[1]][$b[0]] = 'b';
//print_r($mat);
  for( ; $v<36 ; $v++ ) {
    $count = 0;
    for( $y=0 ; $y<$DIMENSION ; $y++ )
    for( $x=0 ; $x<$DIMENSION ; $x++ ) {
      if ( $board[$y][$x]==='b' ) continue;
      if ( $mat[$y][$x]==$v /* && $v>=max(abs($x-$b[0]),abs($y-$b[1]))*/ ) {
	for( $i=$x-1 ; $i<=$x+1 ; $i+=1 ) {
    	  if ( $i<0 ) continue;
    	  if ( $i>=$DIMENSION ) break;
    	  for( $j=$y-1 ; $j<=$y+1 ; $j+=1 ) {
	    if ( $j<0 ) continue;
	    if ( $j>=$DIMENSION ) break;
      	    if ( $i==$x && $j==$y ) continue;
	    if ( $board[$j][$i]==='b' ) continue;
//echo "p1: x y $x, $y, ij= $i $j ". $mat[$j][$i] . ":\n";
      	    if ( is_null($mat[$j][$i]) || $mat[$j][$i]>$v+1 ) {
//echo "p2: x y $x, $y, ij= $i $j\n";
		$mat[$j][$i] = $v+1;
		$count++;
	    }
    	  }
  	}
      }
    }
    if ( $count==0 ) break;
  }
/*
echo "no fim ";
print_r($mat);
 exit;
*/
}

function TopMax(&$b,&$mat1,&$mat2,$white,$player)
{
global $steps1,  $steps2;
  global $DIMENSION;
//return 0;

//  $mat1[$DIMENSION-1][0] = 0;
//  UpdateDijkstraMat($b,$mat1,$white);
  $steps1 = $mat1[$white[1]][$white[0]];
  if ( is_null($steps1) ) $steps1 = 40;

//  $mat2[0][$DIMENSION-1] = 0;
  //UpdateDijkstraMat($b,$mat2,$white);
  $steps2 = $mat2[$white[1]][$white[0]];
  if ( is_null($steps2) ) $steps2 = 40;

//echo "stp1: $steps1 $steps2 ";
//print_r($b);

  if ( $player )  return $steps1-$steps2;
  return $steps2-$steps1;
}

function Valor($board,$mat1,$mat2,$white,$player,$level,$max)
{
global $steps1, $steps2;
  global $DIMENSION;
  global $best;
  $win = GetWinner($white);

$max = -100000;
  if ( $win!=="" ) {
    if ($win===$player )	return  100;
    else			return -100;  
  }
  if ( GetBlocked($board,$white) ) 
    return -100;

  // Eminent Win
  if ( $player==0 ) {
	if ( $white[0]<=1 && $white[1]>=$DIMENSION-2 ) {
		if ($level==0)	$best = array(0,$DIMENSION-1);
		return 98;
	}
  }
  else {
        if ( $white[0]>=$DIMENSION-2 && $white[1]<=0 ) {
                if ($level==0)	$best = array($DIMENSION-1,0);
                return 98;
        }
  }

  $sum = 0;
  $sum = TopMax($board,$mat1,$mat2,$white,$player);
if ( $_GET['debug']>=1 ) {
  echo "<p>L $level, P $player, xy " . $white[0] . $white[1] .", Sum: $sum</p>\n";
  print_r($mat1);
  print_r($mat2);
}
  if ($sum+100<$max) return $sum;

//  echo "stps $steps1 $steps2 sum:$sum ";

  if ( $level>$_SESSION['max_level']+($_SESSION['nplayed']/7) ) 
      return $sum;
//    return TopMax($board,$white,$player);
  //$max = -100000000;
  //$sum = 0;
  //if ($level==0) echo "stps $steps1 $steps2 sum:$sum ";

  $nmat1 = $mat1;
  $nmat2 = $mat2;
  //$nmat1[$white[1]][$white[0]] = 40;
  //$nmat2[$white[1]][$white[0]] = 40;
  $nboard = $board;
  $nboard[$white[1]][$white[0]] = 'b';
  UpdateDijkstraMat($nboard,$nmat1,$white);
  UpdateDijkstraMat($nboard,$nmat2,$white);
  for( $y=$white[1]+1-$player-$player ; $y>=$white[1]-1 && $y<=$white[1]+1 ; $y-=1-$player-$player ) {
    if ( $y<0 || $y>=$DIMENSION ) continue;
    for( $x=$white[0]-1+$player+$player ; $x>=$white[0]-1 && $x<=$white[0]+1 ; $x+=1-$player-$player ) {
      if ( $x<0 || $x>=$DIMENSION) continue;
      if ( $board[$y][$x]=='b' ) continue;
      if ( $x==$white[0] && $y==$white[1] ) continue;

      if ( $_SESSION['symmetric'] && $level==0 ) {
	if ( $x==$white[0] && $y==$white[1]+1 ) continue;
        if ( $x==$white[0]+1 && $y==$white[1] ) continue;
        if ( $x==$white[0]+1 && $y==$white[1]+1 ) continue;
      }

      $nval = -Valor($nboard,$nmat1,$nmat2,array($x,$y),(1-$player),($level+1),-$max)+1;
      $sum += $nval;

//if ( $level==0 )
	//echo "#xy $x $y $nval $sum\n";

      if ( $nval>$max ) {
	$max = $nval;
	if ( $level==0 ) $best = array($x,$y);
	//$best[$level] = array($x,$y);
      }
      if ( $max>100 ) return ($max*0.98+$sum*0.002);
    }
  }
  return ($max*0.98+$sum*0.002);
}

function GetBestPHP() {
  global $best;
  global $bestval;
  global $DIMENSION;
  $best = "";

  $bestval = Valor($_SESSION['board'], $_SESSION['mat1'],$_SESSION['mat2'],$_SESSION['white'], $_SESSION['jogador'], 0, -10000000);

  if ( isset($_GET['debug']) ) {
   print "<p>mat1</p>";
   print_r($_SESSION['mat1']);
   print "<p>mat2</p>";
   print_r($_SESSION['mat2']);
  }
  //echo "<p>Best move: ". chr($best[0]+ord('a')) . ($DIMENSION-$best[1]) . " : " . ($val) . "</p>";
}

function GetBestC($level="") {
  global $best;
  global $bestval;

  if ( $level==='c' || $level<0 ) $level = "";
  $full = "";
  if ( $level=='f' ) {
	$full = "-f ";
	$level = "";
  }
  $res = shell_exec("/home/docentes/jml/WWW/slimetrail/slimetrail -x $full".  GetSTBD($_SESSION['board'],$_SESSION['white'],$_SESSION['jogador']) . " $level" );
  if ( $full=='-f' ) echo $res;
  else  sscanf($res,"%2x%2x %f %d",$best[0],$best[1],$bestval,$bestlevel);
}

function GetBest($level="c")
{
  if ( $level=='h' )	return GetBestPHP();
  return GetBestC($level);
}

function Init() {
    global $DIMENSION;

    $white = array(floor($DIMENSION/2+0.5),floor($DIMENSION/2-1.5));
    //echo "reset";
    unset($_SESSION['board']);
    unset($_SESSION['mat1']);
    unset($_SESSION['mat2']);
    $_SESSION['board'] = array();
    for( $y=0 ; $y<$DIMENSION ; $y++ )
    for( $x=0 ; $x<$DIMENSION ; $x++ )
        $_SESSION['board'][$y][$x] = "";

    $_SESSION['mat1'] = array();
    $_SESSION['mat2'] = array();
    //$_SESSION['mat1'][$DIMENSION-1][0] = 0;
    CreateDijkstraMat($_SESSION['mat1'],array(0,$DIMENSION-1));
    //UpdateDijkstraMat($_SESSION['board'],$_SESSION['mat1'],array(0,$DIMENSION-1));
    //$_SESSION['mat2'][0][$DIMENSION-1] = 0;
    CreateDijkstraMat($_SESSION['mat2'],array($DIMENSION-1,0));

    //UpdateDijkstraMat($_SESSION['board'],$_SESSION['mat2'],array($DIMENSION-1,0));
    $_SESSION['white'] = $white;
    $_SESSION['jogador'] = 0;
    $_SESSION['symmetric'] = 1;
    $_SESSION['player_0'] = 'Local';
    $_SESSION['player_1'] = 'Local';
    $_SESSION['max_level'] = 5;
    $_SESSION['nplayed'] = 0;
/*
if ( isset($_GET['debug']) ) {
 print "<p>mat1</p>";
 print_r($_SESSION['mat1']);
 print "<p>mat2</p>";
 print_r($_SESSION['mat2']);
 exit;
}
*/
}
  if ( !is_array($_SESSION['board']) ) Init();

  $board = $_SESSION['board'];
  $white = $_SESSION['white'];
  if ( isset($_GET['init']) && $_GET['init']>0 ) {
    Init();
    header("Location: " . $_SERVER['PHP_SELF'] );
  }

//echo "<span style='color: gray; font-size: 8pt'>STBD: " . GetSTBD($board,$white,$_SESSION['jogador']) . "</span>\n";

  if ( isset($_GET['p0']) && $_GET['p0']=='t' ) {
	if ( $_SESSION['player_0']=='Local' ) $_SESSION['player_0']='Computer';
	else  $_SESSION['player_0']='Local';
	 header("Location: " . $_SERVER['PHP_SELF'] );
  }
  if ( isset($_GET['p1']) && $_GET['p1']=='t' ) {
        if ( $_SESSION['player_1']=='Local' ) $_SESSION['player_1']='Computer';
        else  $_SESSION['player_1']='Local';
	 header("Location: " . $_SERVER['PHP_SELF'] );
  }

  if ( isset($_GET['x']) &&  isset($_GET['y']) && $_GET['x']!="" && $_GET['y']!="" && $_GET['x']>=0 && $_GET['y']>=0 ) {
	$x = $white[0];
	$y = $white[1];
	if ( abs($x-$_GET['x'])<=1 &&  abs($x-$_GET['x'])<=1 ) {	
	  $board[$y][$x] = 'b';
	  //$_SESSION['mat1'][$y][$x] = 40;
	  //$_SESSION['mat2'][$y][$x] = 40;
	  UpdateDijkstraMat($board,$_SESSION['mat1'],$white);
	  UpdateDijkstraMat($board,$_SESSION['mat2'],$white);
	  if ( ($x+$y+1)!=$DIMENSION ) $_SESSION['symmetric'] = 0;
	  $white[0] = $_GET['x'];
	  $white[1] = $_GET['y'];
	  $_SESSION['board'] = $board;
	  $_SESSION['white'] = $white;
	  $_SESSION['jogador'] = (1-$_SESSION['jogador']);
	  $_SESSION['nplayed'] += 1;
    	}
    header("Location: " . $_SERVER['PHP_SELF'] );
  }

  $vencedor = GetWinner($white);
  if ( $vencedor==="" && GetBlocked($board,$white) )
	$vencedor = 1-$_SESSION['jogador'];

  //print_r($white);
  //print_r($board); //[6][6] = 'b';
?>
<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN"
   "http://www.w3.org/TR/html4/loose.dtd">
<html>
  <head>
    <title>SlimeTrail</title>
    <link href="slimetrail.css"
                type="text/css" rel="stylesheet" >
    <meta http-equiv="content-type" content="text/html;charset=iso-8859-1" >
    <meta name="viewport" content="width=490">
    <script language="javascript" type="text/javascript">
      function PlayCell(x,y)
      {
	    var navlink = "square".concat(x,y);
            document.getElementById(navlink).className = 'white';
	    var dest = "/~jml/slimetrail/index.php?x=".concat(x,"&y=",y);
	    window.location.href=dest;
      }
    </script>
  </head>
  <body>
    <h1>SlimeTrail</h1>

<?php

if ( $vencedor==="" ) {
  if ( $_GET['hint']!="" ) {
    GetBest( $_GET['hint'] );
  }
  else {
	if ( $_SESSION['jogador']==0 && $_SESSION['player_0']=='Computer' ) GetBest();
	if ( $_SESSION['jogador']==1 && $_SESSION['player_1']=='Computer' ) GetBest();
  }
}



  echo "<table>\n  <tr>\n";
  if ( $_SESSION['jogador']>0 ) 
	echo "<td> Jogador 1 </td> <th> Jogador 2 </th>\n";
  else
	echo "<th> Jogador 1 </th> <td> Jogador 2 </td>\n";
  echo "  </tr>\n  <tr>\n";
  echo "   <td class='playertype'> ".
	"<a href='?p0=t'>". $_SESSION['player_0'] ."</a></td>\n";
  echo "   <td class='playertype'> ".
	"<a href='?p1=t'>". $_SESSION['player_1'] ."</a></td>\n";
  echo "  </tr>\n</table>\n";
//  $verdes = 0;
  echo "<table class='board'>\n";
  for( $y=0 ; $y<$DIMENSION ; $y++ ) {
    echo "  <tr>\n";
    echo "    <td class='legend_y'>" . ($DIMENSION-$y) . "</td>\n";
    for( $x=0 ; $x<$DIMENSION ; $x++ ) {
      $class = "square";
      $action ="";
      $id = "square" . $x . $y;
      if ( $board[$y][$x]=='b' ) $class="black";
      else if ( $x==$white[0] && $y==$white[1] ) $class="white";
      else if ( $vencedor=="" && abs($x-$white[0])<=1 && abs($y-$white[1])<=1 ) {
	$class = "active";
	//$action = "onclick='window.location.href=\"" .  $_SERVER['PHP_SELF'] . "?x=$x&amp;y=$y\";'";
	$action = "onclick='PlayCell($x,$y);'";
      }
      echo "    <td id='$id' class='$class' $action> ";
      echo " </td>\n";
    }
    echo "  </tr>\n";
  }
  echo "  <tr>\n";
  echo "    <td class='legend_y'> </td>\n";
  for( $x=0 ; $x<$DIMENSION ; $x++ ) {
    echo "    <td class='legend'>" . chr($x+ord('a')) . "</td>\n";
  }
  echo "  </tr>\n";
  
  echo "</table>\n";

//print_r($_SESSION['board']);
//GetBest();

if ( $vencedor!=="" )
	echo "<h2>O jogador " . ($vencedor+1) . " Venceu!</h2>\n";
else {
  if ( $_GET['hint']!="" ) {
	//if ( $_GET['debug']>=1 ) print_r($best);
	echo "<p>Best move: ". chr($best[0]+ord('a')) . ($DIMENSION-$best[1]) . " : " . round($bestval,2) . "</p>";
  }
  if ( ( $_SESSION['jogador']==0 && $_SESSION['player_0']=='Computer' ) ||
        ( $_SESSION['jogador']==1 && $_SESSION['player_1']=='Computer' ) ) 
	header("Location: ". $_SERVER['PHP_SELF'] . "?x=" . $best[0] . "&y=". $best[1]);
}
?>

<a href='?init=7'>[Recome&ccedil;ar]</a>
<a href='?hint=c'>[Ajuda]</a>
<?php
echo "<br><br><span style='color: gray; font-size: 8pt'>STBD: " . GetSTBD($board,$white,$_SESSION['jogador']) . "</span>\n";

?>
  </body>
</html>
