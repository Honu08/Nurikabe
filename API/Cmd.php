<?php
  class Cmd{
    
    public static function solve($input){
      $output = shell_exec('./clingo '.$input.' rules1.sm | mkatoms');
      $data = array();
      
      $x='';
      $y='';

      //get the values X and Y of the answer
      for($i=0; $i< strlen($output); $i++){
        if($output[$i] == ','){
          $x=$output[$i-1];
          $y=$output[$i+1];
          array_push($data, ''.$x.''.$y);
        }
      }
      return $data;
    }
  }
?>
