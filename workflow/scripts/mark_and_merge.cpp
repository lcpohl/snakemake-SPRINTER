#include <htslib/sam.h>
#include <iostream>
#include <string>
#include <set>
#include <filesystem>
#include <boost/program_options.hpp>
#include <stdexcept>
#include <memory>
#include <unordered_map>
#include <fstream>

namespace po = boost::program_options;

//necessary for habdling bam record pointers properly
struct BAMRecordDeleter {
    void operator()(bam1_t* b) { bam_destroy1(b); }
};

bool record_in_target_refs(bam1_t* record,bam_hdr_t* header,std::unordered_map<std::string,std::string> &accepted_refs){
    int32_t ref_id = record->core.tid;
    if (ref_id < 0||ref_id >= header->n_targets){
        std::cout << "Invalid target\n";
        return false;
    }
    std::string t_name = header->target_name[ref_id];
    return accepted_refs.contains(t_name);
}


int main(int argc, char* argv[]) {
    
    try {
        int state = 0;
        //variables for parameter storing
        std::string input_dir;
        std::string output_file;
		std::string samplesheet;
		std::string refmap;
        uint16_t required_flag;
        uint16_t exclude_flag;
        

        po::options_description desc("Allowed options");
        desc.add_options()
            ("help,h", "Produce help message")
            ("input,i", po::value<std::string>(&input_dir), "Path to directory containing the input bam files")
            ("output,o", po::value<std::string>(&output_file), "Path to output file")
			("samplesheet,s",po::value<std::string>(&samplesheet), "Path to tsv file matching filenames to barcodes. 1-line header expected, 1st col is name, 2nd is barcode")
			("refmap,r",po::value<std::string>(&refmap), "Path to a tsv file containing the names of reference sequences to use. Optionally can include a second column containing replacement names, 1-line header expected")
            ("required_flags",po::value<uint16_t>(&required_flag)->default_value(2),"Only include reads that have all given flags set. Set to 0 to disable, set to 1 to remove requirement for proper pair, set to 2(default) to require reads in proper pairs")
            ("excluded_flags",po::value<uint16_t>(&exclude_flag)->default_value(2052),"Exclude all mappings with any of the given flags set. 2052(default) removes all unmapped reads and supplementary alignments");

        po::variables_map vm;
        po::store(po::parse_command_line(argc, argv, desc), vm);

        if (vm.count("help")) {
            std::cout << "This tool can be used to produce a single barcoded bamfile from a directory containing non-barcoded, single-cell bamfiles. The bamfiles must follow the naming scheme {sample}_{slx}_[cell_no]_{name}-{barcode}.bam, with the name being arbitrary and the cell number optional (only one _ in case of no cell number)\n\n";
            std::cout << desc << "\n";
            return 0;
        }

        po::notify(vm);
        //create filename regex and buffers
        std::string cell_barcode;
        std::string filename;
        std::cout << "Argparse done\n";
		//read samplesheet
		std::unordered_map<std::string,std::string> file_barcodes;
		std::ifstream samplesheet_handle(samplesheet);
		std::string line;
		std::string col1;
		std::string col2;
		size_t delim_loc;
		getline(samplesheet_handle,line);
		while (getline(samplesheet_handle,line)){
			delim_loc = line.find('\t');
			col1 = line.substr(0,delim_loc);
			col2 = line.substr(delim_loc+1);
			file_barcodes[col1] = col2;
		}
		//read barcodes file
		std::unordered_map<std::string,std::string> refnames;
		std::ifstream refnames_handle(refmap);
		getline(refnames_handle,line);
		while (getline(refnames_handle,line)){
            std::cout << "db1 " << line << "\n";
			delim_loc = line.find('\t');
			if (delim_loc != std::string::npos){ //two-column case
				col1 = line.substr(0,delim_loc);
				col2 = line.substr(delim_loc+1);
				refnames[col1] = col2;
                std::cout << col1 <<" / "<<col2<<"\n";
			} else { //one-column case
				col1 = line.substr(0);
				refnames[col1] = col1;
			}	
		}
		//create variables for refname changing and removal
		kstring_t tagbuffer = {0, 0, NULL};
		std::unordered_map<int,int> tid_map;
		sam_hdr_t* new_header;
		sam_hdr_t* header;
        //create filehandles, open output file in write mode and initialise state/buffer variables for the loop
        samFile* infile;
        samFile* outfile;      
        std::cout << output_file << "\n";
        outfile = sam_open(output_file.c_str(),"wb");

		
        std::string filepath;
        bool first = true;
        uint16_t flag;
        std::unique_ptr<bam1_t, BAMRecordDeleter> current_record(bam_init1());

        for (std::filesystem::directory_entry currrent_file : std::filesystem::directory_iterator(input_dir)){ //loop through all files in the given directory
            filepath = currrent_file.path().string();
            filename = filepath.substr(filepath.find_last_of('/')+1);
            if(file_barcodes.contains(filename)){ //only process files in the samplesheet
                cell_barcode = file_barcodes[filename];
                std::cout << filepath << "\n";
                infile = sam_open(filepath.c_str(),"r");
                if (!infile){
                    std::cout << "Couldn't open infile " << filename << " !\n";
                }
                //get header information for the current file
                header = sam_hdr_read(infile);
                if (first){ //copy the header of the first file to the output file
					//replace refnames
					new_header = sam_hdr_dup(header);
					int nseq = sam_hdr_nref(new_header);
					for (int i=0;i<nseq;++i) {
						const char* old_name = header->target_name[i];
                        std::cout << old_name << "\n";
						std::unordered_map<std::string,std::string>::iterator it = refnames.find(old_name);
						if (sam_hdr_find_line_id(new_header,"SQ","SN",old_name,&tagbuffer) < 0){
							std::cerr<<"Cannot find @SQ line for "<<old_name<<"\n";
							return 4;
						}
						if (it==refnames.end()){ 
							//remove reference sequence from header
							if(sam_hdr_remove_line_id(new_header,"SQ","SN",old_name) < 0) {
								std::cerr<<"sam_hdr_remove_line_id failed for "<<old_name<<"\n";
							return 4;
							} else {
                                std::cout << "sam_hdr_remove sucessfull for "<<old_name<<"\n";
                            }
						} else {
							//Update reference sequence entry
							if (sam_hdr_update_line(new_header,"SQ", "SN", old_name,"SN", it->second.c_str(),NULL) < 0) {
								std::cerr<<"sam_hdr_update_line failed for "<<old_name<<"\n";
								return 4;
							} else {
                                std::cout << "sam_hdr_update_line sucessfull for "<<old_name<<"; renamed to "<<it->second<<"\n";
                            }
						}
						
					}
                    state = sam_hdr_write(outfile,new_header);
                    if (state != 0){
                        std::cout << "Writing the header didn't work\n";
                        return - state;
                    }
                    first = false;
                }
				// build a tid -> tid map
				tid_map.clear();
				int nseq = sam_hdr_nref(header);
				for (int i=0;i<nseq;++i) {
					const char* old_name = header->target_name[i];
					std::unordered_map<std::string,std::string>::iterator it = refnames.find(old_name);
					if (it == refnames.end()) {
						continue;
					}
					tid_map[sam_hdr_name2tid(header,old_name)] = sam_hdr_name2tid(new_header,it->second.c_str());
				}
                while (sam_read1(infile,header,current_record.get()) >= 0){ //loop through all records
                    flag =current_record.get()->core.flag;
                    if (((flag & exclude_flag)!=0)||((flag & required_flag)!=required_flag)){ //disregard record with flags not matching the requirements
                        continue;
                    }
					//update tid & tmid
                    if (tid_map.contains(current_record.get()->core.tid) && tid_map.contains(current_record.get()->core.mtid)) {
						current_record.get()->core.tid = tid_map[current_record.get()->core.tid];
						current_record.get()->core.mtid = tid_map[current_record.get()->core.mtid];
					} else {
                        continue;
                    }
                    //update barcode
                    bam_aux_update_str(current_record.get(),"CB",cell_barcode.size(),cell_barcode.c_str());
                    //write to outfile
                    if (sam_write1(outfile,new_header,current_record.get()) < 0){
                        std::cout << "Something went wrong writing a record to the outfile\n";
                    }
                }
                if (infile) {
                    sam_close(infile);
                    sam_hdr_destroy(header);
                }
                std::cout << "Done parsing file "<<filename<<" ,CB set to: "<<cell_barcode << "\n";
            } else {
                std::cout << "File: " << filename << " not present in samplesheet\n";
            }
        }
        std::cout << "End of loop reached\n";
        if(outfile){
            sam_close(outfile);
            sam_hdr_destroy(new_header);
        }
        
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
