<?php

    class File {
        
        public $file;
        public $directory;
        public $extension;
        public $handle;
                        
        public function __construct ($directory, $file, $extension) {
            $this->file      = $file;
            $this->directory = $directory;
            $this->extension = $extension;
        }
        
        public function createFile () {
            $this->handle = null;
            $this->handle = @fopen ($this->getFileName (), "w+");
        }
		
        public function openFileForReading () {
            $this->handle = null;
            $this->handle = @fopen ($this->getFileName (), "r+");
        }
        
        public function openFileForWriting () {
            $this->handle = null;
            $this->handle = @fopen ($this->getFileName (), "w+");
        }
        public function closeFile () {
            $fclose = fclose ($this->handle);
            if ($fclose) {
                return array ("success" => true);
            } else {
                return array ("success" => false);
            }
        }
        
        /*
         *  Deletes file
         *  - returns bool; true if file was deleted successfully, false if not
         */
        public function deleteFile () {
            if ($this->closeFile ()) {
                $unlink = unlink ($this->getFileName ());           
                
                if($unlink) {
                    return array ("success" => true);
                } else {
                    return array ("success" => false);
                }
            } else {
                return array ("success" => false);
            }
        }
		
		 public function delFile ($path) {
                $unlink = unlink ($path);           
                if($unlink) {
                    return array ("success" => true);
                } else {
                    return array ("success" => false);
                }
        }
        
        /*
         *  Reads contents from file
         *  - returns associative array with contents
         */
        public function readFromFile () {
            $this->openFileForReading ();
            $contents = fread ($this->handle, filesize($this->getFileName ()));
            
            if($this->extension == "json") {
                return (array) json_decode ($contents);
            } else {
                return $contents;
            }
        }
		
        /*
         *  Writes contents to file
         *  - returns bool; true if wrote to file successfully, false if not
         */
        public function writeToFile ($data) {
            $this->openFileForWriting ();
            $response = fwrite ($this->handle, json_encode ($data, JSON_PRETTY_PRINT));
                        
            if($response) {
                return array ("success" => true);
            } else {
                return array ("success" => false);
            }
        }
        
        /*
         *  Creates directory if it doesn't exist
         *  - returns bool; true if created successfully, false if not
         */
		public static function createDirectory ($path) {
			if (!self::fileExists ($path)) {
            	return @mkdir ($path);
			} else {
				return array ( "success" => false, "message" => "Directory already exists" );
			}
        }
		
        /*
         *  Gets contents within directory
         *  - returns array with file names
         */
        public static function listFilesInsidePath ($path) {
            return scandir ($path);
        }
        
        /*
         *  Verifies the existence of a specific file or directory
         *  - returns bool
         */
        public static function fileExists ($directory, $file, $extension) {
            $instance = new self ($directory, $file, $extension);
            if (file_exists ($instance->getFileName ())) {
                return true;
            } else {
                return false;
            }
        }
                
        /*
         *  Concatenates directoy, file, and extension
         *  - returns string with directory file path
         */
        public function getFileName () {
            return $this->directory . $this->file . "." . $this->extension;
        }
    }

?>