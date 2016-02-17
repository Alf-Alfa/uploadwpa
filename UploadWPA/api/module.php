<?php namespace pineapple;

class UploadWPA extends Module
{
	public function route()
	{
		switch ($this->request->action)
		{
			//When it get's accepted as an official module, I can uncomment this...
			/*
			case 'getDependencyStatus':
				$this->getDependencyStatus();
				break;
			case 'handleUploadWPADepends':
				$this->getDependencyStatus();
				break;
			*/
			case 'loadTempDataIfExists':
				$this->loadTempDataIfExists();
				break;

			case 'sendWPAHandshake':
				$this->sendWPAHandshake();
				break;
		}
	}

	private function loadTempDataIfExists()
	{
		$tempEmailAddress = file_get_contents('/tmp/uploadwpa_temp');
		if($tempEmailAddress !== false)
		{
			$this->response = $tempEmailAddress;
		}
		else
		{
			$this->error = "No temporarily saved email address to retrieve...";
		}
	}
	
	private function sendWPAHandshake()
	{
		if(empty($this->request->email) && empty($this->request->wpafile))
		{
			$this->error = "Please specify your email address & path to wpa handshake capture file to upload...";
		}
		else
		{
			file_put_contents('/tmp/uploadwpa_temp', $this->request->email . "\n");

			$uploadwpaCommand = "uploadwpa -e " . $this->request->email . " -c " . $this->request->wpafile;
			$this->response = array("commandResult" => exec($uploadwpaCommand));
		}
	}

	private function getDependencyStatus()
	{
		$this->response = array("uploadwpa" => $this->checkDependency("uploadwpa"));
	}

	private function handleUploadWPADepends()
	{
		$response_array = array();
		$installed = $this->checkDependency("uploadwpa");

		if(!$installed)
		{
			$success = $this->installDependency("uploadwpa");
			$message = "Successfully installed dependencies.";
			if (!$success)
			{
				$message = "Error occured while installing 'uploadwpa'";
			}
			$response_array = array("control_succcess" => $success, "control_message" => $message);
		}
		else
		{
			exec("opkg remove uploadwpa");
			$removed = !$this->checkDependency("uploadwpa");
			$message = "Successfully removed dependencies.";
			if(!$removed)
			{
				$message = "Error removing dependencies.";
			}
			$response_array = array("control_succcess" => $removed, "control_message" => $message);
		}
		$this->response = $response_array;
	}
}
