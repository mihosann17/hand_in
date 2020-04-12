#!/usr/local/bin/perl
#

if($#ARGV eq -1)
{
  @ARGV=("A","B","C","D","E","F","G","H");
}elsif($#ARGV>=8){
  print"Error";
  Exit;
}

@alph=@ARGV;
$alp=join('',@alph);

open(FH,"> group-$alp.tex");
print (FH"\\documentclass{jsarticle}\n");
print (FH"\\usepackage{multirow}\n");
print (FH"\\begin{document}\n");
close FH;

foreach  $item(@alph){
  open(FILE,"group$item");
#ファイルデータ一行ごとの配列を作成
  while($DATA=<FILE>){
    chomp($DATA);
    push(@sente,$DATA);
  }
  close FILE;
  foreach $x(@sente){
    @tmp=split(/\s+/,$x);
    push(@word,@tmp);#一単語ごとの配列を作成
    foreach $k(@tmp){
      if($k eq "W"){
        $w++;
      }
      elsif($k eq "L"){
        $l++;
      }
      elsif($k eq "D"){
        $d++;
      }      
    }
    push(@win,$w);
    push(@lose,$l);
    push(@draw,$d);
      $w=0;
      $d=0;
      $l=0;
  }

#W,L,Dの書き換え
  foreach $y(@word){
    $y=~ s/_/-/g;

    if($y eq "W"){
      $y="\○";
    }
    elsif($y eq "L"){
      $y="\×";
    }
    elsif($y eq "D"){
      $y="\△";
    }
  }

@point=($win[4]*3+$draw[4],$win[5]*3+$draw[5],$win[6]*3+$draw[6],$win[7]*3+$draw[7]);

#TeXファイルのへ入力
  open(FP,">> group-$alp.tex");
  print FP <<"EOS";
\\begin{table}[htb]
\\begin{center}
\\caption{$sente[0]-$sente[2]}
\\begin{tabular}{|c|c|c|c|c|c|c|c|c|}\\hline
          \& $word[9] \& $word[10]\& $word[11] \& $word[12]\&勝ち数\&引き分け数\&負け数\&勝ち点　   \\\\ \\hline
\\multirow{2}{*}{$word[13]} \&\\multirow{2}{*}{}\& $word[15]\& $word[17] \& $word[19]&\\multirow{2}{*}{$win[4]}\&\\multirow{2}{*}{$draw[4]}\&\\multirow{2}{*}{$lose[4]}\&\\multirow{2}{*}{$point[0]}\\\\ 
                      \&          \& $word[16]\& $word[18] \& $word[20]\&\&\&\&      \\\\ \\hline
\\multirow{2}{*}{$word[21]} \& $word[22]\& \\multirow{2}{*}{} \& $word[25] \& $word[27]\& \\multirow{2}{*}{$win[5]}\& \\multirow{2}{*}{$draw[5]}\& \\multirow{2}{*}{$lose[5]}\&\\multirow{2}{*}{$point[1]}\\\\ 
          \& $word[23]\&          \& $word[26] \& $word[28]\&\&\&\&      \\\\ \\hline
\\multirow{2}{*}{$word[29]} \& $word[30]\& $word[32]\&\\multirow{2}{*}{} \& $word[35]\& \\multirow{2}{*}{$win[6]}\& \\multirow{2}{*}{$draw[6]}\& \\multirow{2}{*}{$lose[6]}\& \\multirow{2}{*}{$point[2]}\\\\ 
          \& $word[31]\& $word[33]\&           \& $word[36]\&\&\&\&      \\\\ \\hline
\\multirow{2}{*}{$word[37]} \& $word[38]\& $word[40]\& $word[42] \& \\multirow{2}{*}{}\& \\multirow{2}{*}{$win[7]}\& \\multirow{2}{*}{$draw[7]}\& \\multirow{2}{*}{$lose[7]}\& \\multirow{2}{*}{$point[3]}\\\\ 
          \& $word[39]\& $word[41]\& $word[43] \&          \&\&\&\&      \\\\ \\hline
\\end{tabular}
\\end{center}
\\end{table}
\\newpage
EOS
  close FP;
  @sente=();#配列の初期化
  @word=();
  @win=();
  @lose=();
  @draw=();
}

open(FH,">> group-$alp.tex");
print(FH"\\end{document}\n");
close FH;
