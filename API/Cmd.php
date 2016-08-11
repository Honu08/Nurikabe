<?php
  class Cmd{
    
    public static function solve($input){
      
      //execute clingo command and get result in variable
      $output = shell_exec('./clingo '.$input.' rules1.sm | mkatoms');
      
      //array to return
      $data = array();
      
      $x='';
      $y='';

      //get the values X and Y of the answer
      for($i=0; $i< strlen($output); $i++){
        //find commas in answer set output
        if($output[$i] == ','){
          //get value of x
          $x=$output[$i-1];
          //get value of y
          $y=$output[$i+1];
          
          //push the values x and y to the array 
          array_push($data, ''.$x.''.$y);
        }
      }
      return $data;
    }
  }
?>
