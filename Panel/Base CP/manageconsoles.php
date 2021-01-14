<?php
include "includes/settings.php";
include "includes/database.php";

if(!isset($_SESSION))
{
    session_start();
}

if(!isset($_SESSION['username']))
{
    echo'
    <script language="javascript">
    window.location.href="index.php"
    </script>
    ';
}

$consoles = mysqli_query($con, "SELECT COUNT(1) FROM `consoles`");
$consoles_row = mysqli_fetch_array($consoles);
$consoles_total = $consoles_row[0];
$time_now = time("Y-m-d");

if(isset($_GET['deleteconsole']))
{
    if($_GET['deleteconsole'])
    {
        $id = strip_tags($_GET['deleteconsole']);
        mysqli_query($con, "DELETE FROM `consoles` WHERE `id` = '$id'") or die(mysqli_error($con));
    }
}

?>
<!DOCTYPE html>
<html lang="en">
<meta http-equiv="content-type" content="text/html;charset=utf-8" />
<head>        
        <title><?php echo $name; ?> &bull; Consoles</title>    

        <meta http-equiv="Content-Type" content="text/html; charset=utf-8" />        
        <meta http-equiv="X-UA-Compatible" content="IE=edge" />
        <meta name="viewport" content="width=device-width, initial-scale=1" />
        <link href="css/styles.css" rel="stylesheet" type="text/css" />
        <script type="text/javascript" src="js/plugins/jquery/jquery.min.js"></script>
        <script type="text/javascript" src="js/plugins/jquery/jquery-ui.min.js"></script>
        <script type="text/javascript" src="js/plugins/bootstrap/bootstrap.min.js"></script>
        <script type="text/javascript" src="js/plugins/mcustomscrollbar/jquery.mCustomScrollbar.min.js"></script>            
        <script src="https://maps.googleapis.com/maps/api/js?v=3.exp&amp;sensor=false&amp;libraries=places"></script>
        <script type="text/javascript" src="js/plugins/fancybox/jquery.fancybox.pack.js"></script>                
        <script type="text/javascript" src="js/plugins/rickshaw/d3.v3.js"></script>
        <script type="text/javascript" src="js/plugins/rickshaw/rickshaw.min.js"></script>
        <script type='text/javascript' src='js/plugins/knob/jquery.knob.js'></script>
        <script type="text/javascript" src="js/plugins/daterangepicker/moment.min.js"></script>
        <script type="text/javascript" src="js/plugins/daterangepicker/daterangepicker.js"></script> 
        <script type='text/javascript' src='js/plugins/jvectormap/jquery-jvectormap-1.2.2.min.js'></script>
        <script type='text/javascript' src='js/plugins/jvectormap/jquery-jvectormap-europe-mill-en.js'></script>
        <script type="text/javascript" src="js/plugins.js"></script>        
        <script type="text/javascript" src="js/demo.js"></script>
        <script type="text/javascript" src="js/maps.js"></script>        
        <script type="text/javascript" src="js/charts.js"></script>
        <script type="text/javascript" src="js/actions.js"></script>    
    
        
    </head>
    <body>
        
        <div class="page-container">
            <div class="page-navigation">
                
                <div class="profile">                    
                    <img src="img/samples/users/xbox.png"/>
                    <div class="profile-info">
                        <a class="profile-title"><?php echo $_SESSION['username']; ?></a>                      
                    </div>
                </div>
                <ul class="navigation">
                    <li class="active"><a href="javascript:void"><i class="fa fa-users"></i> Manage Consoles</a></li>
                    <li><a href="failed.php"><i class="fa fa-ban"></i> Failed Consoles</a></li>
					<li><a href="settings.php"><i class="fa fa-gear"></i> Settings</a></li>
					<li><a href="lib/logout.php"><i class="fa fa-gear"></i> Logout</a></li>
                    </li>                    
                </ul>
                
            </div>
            
            <div class="page-content">
                <div class="container">                        
					<div class="row">
						<div class="col-md-10">
						<?php
						if(isset($_POST['check']))
						{
							$cpukey = $_POST['cpukeyVal'];
							$get_time = mysqli_query($con, "SELECT * FROM `consoles` WHERE `cpukey` = '".$cpukey."'");
							$do_time_check = mysqli_num_rows($get_time);
							$row = mysqli_fetch_array($get_time);
							$time = $row['time'];
							$consoles_name = $row['name'];
							$realx_time=strtotime($time) - strtotime('now');
							$real_time = intval($realx_time/60/60/24);

							if($do_time_check == 1)
							{
								echo '<div class="row"><div class="col-md-12"><div class="alert alert-success">
                                <button type="button" class="close" data-dismiss="alert" aria-hidden="true">×</button>
                                <center><strong>Console found in database.</strong></center> <center>
                                <div>Name : '.$consoles_name.'</div>
                                <div>CPUKey : '.$cpukey.'</div>
                                <div>Remaining Time : '.($real_time == "16606" ? "LIFETIME" : $real_time.'days').  '</div>
                                </center>
                                </div></div></div>';
							}
							elseif($do_time_check == 0)
							{
								echo '<div class="row"><div class="col-md-12"><div class="alert alert-danger">
                                <button type="button" class="close" data-dismiss="alert" aria-hidden="true">×</button>
                                <strong>FAILED</strong> Console not found in database!
                                </div></div></div>';
							}
						}
						?>
							<div class="panel panel-info">
								<form method="POST">
									<div class="panel-heading">
										<h3 class="panel-title">Check Time</h3>
									</div>
									<div class="panel-body controls no-padding">
										<div class="row-form">
											<div class="col-md-3"><strong>CPU Key:</strong></div>
											<div class="col-md-9"><input type="text" class="form-control" required placeholder="cpu key" name="cpukeyVal" maxlength="32"/></div>
										</div>
									</div>
									<div class="panel-footer"><button class="btn btn-success" name="check">Check Time</button></div>
								</form>
							</div>
						</div>
					</div>
					
					
                    <div class="row">
                        <div class="col-md-10">
                        <?php
                                if(isset($_POST["create"]))
                                {
                                    $client_name = $_POST['nameVal'];
                                    $client_cpuk = $_POST['cpukeyVal'];
                                    $client_email = $_POST['emailVal'];
                                    $client_time = $_POST['daysVal'];
                                    $time_now = new datetime(date("Y-m-d"));
                                    $time_now->add(new DateInterval('P'.$client_time.'D'));
                                    $client_realtime = $time_now->format('Y-m-d') . "\n";

                                    $select_username_first = mysqli_query($con, "SELECT COUNT(*) FROM `consoles` WHERE `username` = '$client_name'");
                                    $select_cpukey_first = mysqli_query($con, "SELECT * FROM `consoles` WHERE `cpukey` = '$client_cpuk'");
                                        $select_cpuk_first = mysqli_num_rows($select_cpukey_first);

                                        if($select_cpuk_first > 0)
                                        {
                                            echo '<div class="row"><div class="col-md-12"><div class="alert alert-danger">
                                                <button type="button" class="close" data-dismiss="alert" aria-hidden="true">×</button>
                                                <strong>FAILED</strong> cpukey already exists in database!
                                            </div></div></div>';
                                        }
                                        else
                                        {
                                            $select_usermail_first = mysqli_query($con, "SELECT * FROM `consoles` WHERE `email` = '$client_email'");
                                            $select_email_first = mysqli_num_rows($select_usermail_first);

                                            if($select_email_first > 0)
                                            {
                                                echo '<div class="row"><div class="col-md-12"><div class="alert alert-danger">
                                                <button type="button" class="close" data-dismiss="alert" aria-hidden="true">×</button>
                                                <strong>FAILED</strong> email already exists in database!
                                            </div></div></div>';
                                            }
                                            else
                                            {
                                                $insert = mysqli_query($con, "INSERT INTO `consoles`(`id`, `name`, `cpukey`, `email`, `time`, `enabled`) VALUES ('NULL', '$client_name', '$client_cpuk', '$client_email', '$client_realtime', '1')");

                                        if($insert)
                                        {
                                            echo '<div class="row"><div class="col-md-12"><div class="alert alert-success">
                                            <button type="button" class="close" data-dismiss="alert" aria-hidden="true">×</button>
                                            <strong>SUCCESS</strong> '.$client_name.' added!
                                            </div></div></div>';
                                            echo '<meta http-equiv="refresh" content="3;url=index.php">';
                                        }
                                        elseif(!$insert)
                                        {
                                            echo '<div class="row"><div class="col-md-12"><div class="alert alert-danger">
                                                <button type="button" class="close" data-dismiss="alert" aria-hidden="true">×</button>
                                                <strong>FAILED</strong> console not added!
                                            </div></div></div>';
                                        }
                                        }
                                    }
                                }
                                ?>
                            <div class="panel panel-info">
                                <form method="POST">
                                    <div class="panel-heading">
                                        <h3 class="panel-title">Add Console</h3>
                                    </div>
                                    <div class="panel-body controls no-padding">
                                        <div class="row-form">
                                            <div class="col-md-3"><strong>Name:</strong></div>
                                            <div class="col-md-9"><input type="text" required class="form-control" placeholder="name" name="nameVal"/></div>
                                        </div>
                                        <div class="row-form">
                                            <div class="col-md-3"><strong>CPU Key:</strong></div>
                                            <div class="col-md-9"><input type="text" required class="form-control" maxLength = "32" placeholder="cpu key" name="cpukeyVal"/></div>
                                        </div>
                                        <div class="row-form">
                                            <div class="col-md-3"><strong>Email:</strong></div>
                                            <div class="col-md-9"><input type="email" required class="form-control" placeholder="email" name="emailVal"/></div>
                                        </div>
                                        <div class="row-form">
                                            <div class="col-md-3"><strong>Time:</strong></div>
                                            <div class="col-md-9">
                                                <select class="form-control" name="daysVal">
                                                    <option value="1">1 Day</option>
													<option value="2">2 Day</option>
													<option value="3">3 Day</option>
													<option value="4">4 Day</option>
													<option value="5">5 Day</option>
													<option value="6">6 Day</option>
                                                    <option value="7">1 Week</option>
													<option value="14">2 Week</option>
													<option value="21">3 Week</option>
													<option value="31">1 Month</option>
													<option value="62">2 Months</option>
													<option value="93">3 Months</option>
													<option value="124">4 Months</option>
													<option value="155">5 Months</option>
													<option value="186">6 Months</option>
													<option value="217">7 Months</option>
													<option value="248">8 Months</option>
													<option value="279">9 Months</option>
													<option value="310">10 Months</option>
													<option value="341">11 Months</option>
													<option value="372">1 year</option>
                                                    <option value="36500">Lifetime</option>
                                                </select>
                                            </div>
                                        </div>
                                    </div>
                                    <div class="panel-footer"><button class="btn btn-success" name="create">Add Console</button></div>
                                </form>
                            </div>
                        </div>
                    </div>
                    <div class="row">
                        <div class="col-md-10">
                            <div class="panel panel-info">
                                <div class="panel-heading">
                                    <h3 class="panel-title">Consoles (Total: <?php echo $consoles_total; ?>)</h3>
                                </div>
                                <div class="panel-body controls no-padding">
                                    <div class="block">
                                        <table cellpadding="0" cellspacing="0" width="100%" class="table table-bordered table-striped sortable">
                                            <thead>
                                                <tr>
                                                    <th>Id</th>
                                                    <th>Name</th>
                                                    <th>CPU Key</th>
                                                    <th>Email</th>
													<th>Time</th>
                                                    <th>Enabled</th>
                                                    <th style="width: 180px;"><center>Actions</center></th>
                                                </tr>
                                            </thead>
                                            <tbody>
                                            <tr>
                                            <?php
                                            $consoles_result = mysqli_query($con, "SELECT * FROM `consoles`");
                                            while($consoles_row2 = mysqli_fetch_array($consoles_result))
                                            {
                                                echo '
                                                <tr>
                                                <td>
                                                '.$consoles_row2['id'].'
                                                </td>
                                                <td>
                                                '.$consoles_row2['name'].'
                                                </td>
                                                <td>
                                                '.$consoles_row2['cpukey'].'
                                                </td>
                                                <td>
                                                '.$consoles_row2['email'].'
                                                </td>
												<td>
                                                '.$consoles_row2['time'].'
                                                </td>
                                                <td>
                                                <center>'. ($consoles_row2['enabled'] == "1" ? "<label class = 'label label-success'>Enabled</label>" : "<label class = 'label label-danger'>Disabled</label>") .'</label></center>
                                                </td>
                                                <td>
                                                <form action = "edituser.php?id='.$consoles_row2['id'].'" method="POST">
                                                <center><button type="submit" class="btn btn-primary"><i class="fa fa-cog"></i> Edit Console</button></center>
                                                </form>
                                                </td>'
                                                ;
                                            }
                                            ?>
                                            </tr>
                                            </tbody>
                                        </table>
                                    </div>
                                </div>
                            <br />
                     </div>
                </div>
                
            </div>
            <div class="page-sidebar"></div>
        </div>
    </body>
</html>
