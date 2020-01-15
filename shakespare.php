<?php

include_once "../action/functions.php";

$url="http://shakespeare.mit.edu/index.html";
$cookie="";
$reg="/table.*?<\/table/ism";
$res=fcontents($url,$cookie);
if(preg_match_all($reg,$res,$match)){
    $echo=$match[0][1];
}

$reg_url="/a href=\".*?\"/ism";
if(preg_match_all($reg_url,$echo,$url_match)){
    $url=$url_match[0];
};
$url=cuta($url,8);
$url=cutal($url,1);

$begin=1001;
for($i=0;$i<1;$i++){
    $scene=1;
    $work=fcontents("http://shakespeare.mit.edu/".$url[$i],$cookie);
    if(preg_match_all($reg_url,$work,$sub_match)){
        $sub_url=$sub_match[0];
        $sub_url=cuta($sub_url,8);
        $sub_url=cutal($sub_url,1);
        $sub_url=array_slice($sub_url,3,count($sub_url)-3);
    }
    for($j=0;$j<1;$j++){
        $headUrl=substr($sub_url[$j],0,strlen($sub_url[$j])-9);
        $detail=fcontents("http://shakespeare.mit.edu/".$headUrl."/".$sub_url[$j],$cookie);//ordinary
//        $detail=fcontents("http://shakespeare.mit.edu/Poetry/".$sub_url[$j],$cookie);//poetry
        echo $detail;
        $artical_reg="/3>SCENE.*?<table/ism";
        if(preg_match_all($artical_reg,$detail,$articalmatch)){
            $artical=$articalmatch[0][0];
            $artical=substr($artical,2,count($artical)-7);
            $artical=preg_replace("/<.*?>/ism","",$artical); 
            $file=fopen(($begin+$i)." ".($scene+$j).".txt","w");
            fwrite($file,$artical);
            fclose($file);
        }
    }
}