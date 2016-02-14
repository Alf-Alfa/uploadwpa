registerController('UploadWPAController', ['$api', '$scope', function($api, $scope){
    $scope.emailText = "";
	$scope.wpaFilePath = "";
    $scope.uploadedResult = "";
	$scope.uploadwpaInstalled = false;

	getDependencyStatus();

    $scope.sendWPAHandshake = (function()
	{
		$api.request({
		module: 'UploadWPA',
		action: 'sendWPAHandshake',
		email: $scope.emailText,
		wpafile: $scope.wpaFilePath
        }, function(response){
            if(response.error === undefined)
			{
                $scope.uploadedResult = response.commandResult;
            }
			else
			{
                $scope.uploadedOutput = response.error;
            }
        });
    });

	$scope.uploadwpaInstallUninstall = (function()
	{
		$api.request({
		module: 'UploadWPA',
		action: 'handleUploadWPADepends'
        }, function(response){
            $scope.installedResult = response.control_message;
        });
	});

	function getDependencyStatus()
	{
		$api.request({
			module: "UploadWPA",
			action: "getDependencyStatus"
		}, function(response){
			console.log(response);
			updateDependencyStatus(response);
		});
	}

	function updateDependencyStatus(response)
	{
		$scope.uploadwpaInstalled = false;
		if(response.uploadwpa)
		{
			$scope.uploadwpaInstalled = true;
		}
	}
}]);
