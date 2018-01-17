#include "data.hpp"


static double lower_bound = 1e-10;

Data::Data(int N, int L, int Ne, double mu): N(N), L(L), Ne(Ne), mu(mu){ 
  name     = "relate"; 
}

Data::Data(const char* filename_sequence, const char* filename_pos, const char* filename_rec, const char* filename_rpos, int Ne, double mu): Ne(Ne), mu(mu){
  ReadSequenceFromBin(filename_sequence);
  ReadVectorFromBin(pos, filename_pos);
  ReadVectorFromBin(r, filename_rec);
  ReadVectorFromBin(rpos, filename_rpos);

  name     = "relate"; 
  theta    = 0.025;
  ntheta   = 1.0 - theta;
}

Data::Data(const char* filename_pos, const char* filename_param, int Ne, double mu): Ne(Ne), mu(mu){ 

  FILE* fp = fopen(filename_param, "r");
  assert(fp != NULL);
  fread(&N, sizeof(int), 1, fp);
  fread(&L, sizeof(int), 1, fp);
  fclose(fp); 

  ReadVectorFromBin(pos, filename_pos);

  name     = "relate"; 
  theta    = 0.025;
  ntheta   = 1.0 - theta;
}


/////////////////////////////////////

/*
void 
Data::MakeChunks2(const std::string& filename_haps, const std::string& filename_sample, const std::string& filename_map, const std::string& filename_dist){


  haps m_haps(filename_haps.c_str(), filename_sample.c_str());
  N = m_haps.GetN();
  L = m_haps.GetL();
  std::vector<char>::size_type uN = N; 

  std::vector<int> bp_pos(L+1);
  std::vector<char> ancestral(L), alternative(L);
  std::vector<std::string> rsid(L);
 
  int chunk_size; 
  int num_chunks; 
  int window_length1 = (int) (10.0/4.0 * 1e9/(N*N)) + 1;
  int window_length2 = (int) ((L-1.0)/499.0) + 1;
  if( window_length1 >= window_length2 ){

    chunk_size = L;
    num_chunks = 1;
    window_length = window_length1;

  }else{
  
    int max_num_chunks = L/200000.0 + 1;

    chunk_size    = L/(max_num_chunks - 1.0);
    window_length = (int) ((chunk_size-1.0)/499.0) + 1;
    if( (int) ((chunk_size + window_length-1)/window_length) > 500 || N*N*window_length*4.0/1e9 > 10){
      //this is when data set is theoretically too large for this programme to run on 5GB of memory (but in practise it might be OK).
      num_chunks    = max_num_chunks;
      window_length = 400;
      chunk_size    = 200000;
    }else{

      for(num_chunks = max_num_chunks-1; num_chunks >= 2; num_chunks--){
        chunk_size    = L/(num_chunks - 1.0);
        window_length = (int) ((chunk_size-1.0)/499.0) + 1;
        if( (int) ((chunk_size + window_length-1)/window_length) > 500 || N*N*window_length*4.0/1e9 > 10 ) break;
      }

      num_chunks++;
      chunk_size    = L/(num_chunks - 1.0);
      window_length = (int) ((chunk_size-1.0)/499.0) + 1;

    }
  
  }

  if(window_length > L) window_length = L;

  /////////////////// Output program parameters into file ///////////////
  FILE* fp = fopen("parameters.bin", "w");
  assert(fp);
  fwrite(&N, sizeof(int), 1, fp);
  fwrite(&L, sizeof(int), 1, fp);
  fwrite(&num_chunks, sizeof(int), 1, fp);
  fwrite(&window_length, sizeof(int), 1, fp);
  fclose(fp);

  std::cerr << std::setprecision(2) << "Warning: Will use approx " << 2.0*(4.0 * N * N * (chunk_size/((double)window_length)+2.0))/1e9 << "GB of hard disc." << std::endl;
  std::cerr << "Warning: Will open " << (int) ((chunk_size-1)/window_length) + 1 << " files." << std::endl;

  int overlap = 20000;
  std::vector<std::vector<char> > p_seq(chunk_size);
  std::vector<std::vector<char> >::iterator it_p = p_seq.begin();
  for(; it_p != p_seq.end(); it_p++){
    (*it_p).resize(N);
  }

  int snp = 0;
  for(int chunk = 0; chunk < num_chunks; chunk++){

    FILE* fp_haps_chunk = fopen(("chunk_" + std::to_string(chunk) + ".hap").c_str(), "wb");
    assert(fp_haps_chunk);
    //output chunk bed into fp_haps_chunk and chunk pos, rpos into fp_pos

    if(snp > 0){

      int L_chunk           = std::min(L-snp,chunk_size) + overlap; 
      std::vector<char>::size_type uL_chunk = std::min(L-snp,chunk_size) + overlap; 
      fwrite(&uL_chunk, sizeof(std::vector<char>::size_type), 1, fp_haps_chunk);
      fwrite(&uN, sizeof(std::vector<char>::size_type), 1, fp_haps_chunk);

      /////////////////// Output program parameters into file ///////////////
      FILE* fp = fopen(("parameters_c" + std::to_string(chunk) + ".bin").c_str(), "w");
      assert(fp);
      fwrite(&N, sizeof(int), 1, fp);
      fwrite(&L_chunk, sizeof(int), 1, fp);
      fwrite(&window_length, sizeof(int), 1, fp);
      fclose(fp);

      for(it_p = std::prev(p_seq.end(),overlap); it_p != p_seq.end(); it_p++){
        for(std::vector<char>::iterator it_row = (*it_p).begin(); it_row != (*it_p).end(); it_row++){ 
          fwrite(&(*it_row), sizeof(char), 1, fp_haps_chunk);
        }
      }

    }else{  

      int L_chunk           = std::min(L-snp,chunk_size); 
      std::vector<char>::size_type uL_chunk = std::min(L-snp,chunk_size); 
      fwrite(&uL_chunk, sizeof(std::vector<char>::size_type), 1, fp_haps_chunk);
      fwrite(&uN, sizeof(std::vector<char>::size_type), 1, fp_haps_chunk);

      /////////////////// Output program parameters into file ///////////////
      FILE* fp = fopen(("parameters_c" + std::to_string(chunk) + ".bin").c_str(), "w");
      assert(fp);
      fwrite(&N, sizeof(int), 1, fp);
      fwrite(&L_chunk, sizeof(int), 1, fp);
      fwrite(&window_length, sizeof(int), 1, fp);
      fclose(fp);

    }

    int snp_begin = snp;
    it_p = p_seq.begin();
    for(snp = snp_begin; snp < std::min(L,(chunk+1)*chunk_size); snp++){
      m_haps.ReadSNP(*it_p, bp_pos[snp]);

      ancestral[snp]   = m_haps.ancestral;
      alternative[snp] = m_haps.alternative;
      rsid[snp]        = m_haps.rsid;

      it_p++;
    }

    if(snp-snp_begin < chunk_size){
      p_seq.resize(snp-snp_begin);
    }

    for(it_p = p_seq.begin(); it_p != p_seq.end(); it_p++){
      for(std::vector<char>::iterator it_row = (*it_p).begin(); it_row != (*it_p).end(); it_row++){ 
        fwrite(&(*it_row), sizeof(char), 1, fp_haps_chunk);
      }
    }

    fclose(fp_haps_chunk);

  }
  bp_pos[bp_pos.size()-1] = bp_pos[bp_pos.size()-2] + 1;
  m_haps.CloseFile();

  //////////////////////// calculate rpos, r, pos here
  pos.resize(L); 
  std::vector<int>::iterator it_pos, it_bppos, it_bppos_next;

  if(filename_dist == "unspecified"){

    it_pos = pos.begin();
    it_bppos = bp_pos.begin();
    it_bppos_next = std::next(bp_pos.begin(), 1);
    int count = 0;
    for(; it_pos != std::prev(pos.end(),1);){
      *it_pos = *it_bppos_next - *it_bppos;
      if(*it_pos <= 0){
        std::cerr << "SNPs are not sorted by bp or more than one SNP at same position." << std::endl;
        exit(1);
      }
      it_pos++;
      it_bppos++;
      it_bppos_next++;
      count++;
    }
    pos[pos.size()-1] = 1;

  }else{

     
    char buffer[40];
    FILE* fp_dist = fopen(filename_dist.c_str(), "r");
    fscanf(fp_dist, "%s %s", buffer, buffer);
    int bp_dist, dist;
    snp = 0;
    while(fscanf(fp_dist, "%d %d", &bp_dist, &dist) !=EOF){
      assert(snp < L);
      assert(bp_pos[snp] == bp_dist);
      pos[snp] = dist;
      snp++;
    }
    fclose(fp_dist);
  
  }

  //output props
  FILE* fp_props = fopen("props.bin", "wb"); //storing rsid and other info in this file
  assert(fp_props);

  for(snp = 0; snp < L; snp++){
    fwrite(&snp, sizeof(int), 1, fp_props);
    fwrite(&bp_pos[snp], sizeof(int), 1, fp_props);
    fwrite(&pos[snp], sizeof(int), 1, fp_props);   
    fwrite(&rsid[snp][0], sizeof(char), 30, fp_props);
    fwrite(&ancestral[snp], sizeof(char), 1, fp_props);
    fwrite(&alternative[snp], sizeof(char), 1, fp_props);
  }

  fclose(fp_props);

  //parse map
  map m_map(filename_map.c_str());
  r.resize(L);
  rpos.resize(L+1); 
  std::vector<double>::iterator it_r, it_rpos;
  it_rpos = rpos.begin();
  it_bppos = bp_pos.begin();
  int map_pos = 0;

  if(m_map.bp[map_pos] > *it_bppos){
    *it_rpos = m_map.gen_pos[map_pos] * 1e-2;
    it_rpos++;
    it_bppos++;
  }
  for(; it_rpos != rpos.end();){
    while(m_map.bp[map_pos+1] <= *it_bppos && map_pos < m_map.bp.size()-2) map_pos++;
    if(m_map.bp[map_pos+1] < m_map.bp[map_pos]) std::cerr << m_map.bp[map_pos] << " " << m_map.bp[map_pos+1] << std::endl;
    assert(m_map.bp[map_pos+1] - m_map.bp[map_pos] >= 0);

    if(m_map.bp[map_pos+1] - m_map.bp[map_pos] == 0){
      *it_rpos = m_map.gen_pos[map_pos] * 1e-2;
    }else{
      *it_rpos = ((*it_bppos - m_map.bp[map_pos])/((double)(m_map.bp[map_pos+1] - m_map.bp[map_pos])) * (m_map.gen_pos[map_pos+1] - m_map.gen_pos[map_pos]) +  m_map.gen_pos[map_pos]) * 1e-2;
    }

    it_rpos++;
    it_bppos++;
  }

  it_r = r.begin();
  it_rpos = rpos.begin();
  std::vector<double>::iterator it_rpos_next = std::next(rpos.begin(),1);
  for(; it_r != r.end();){
    *it_r = *it_rpos_next - *it_rpos;
    if(*it_r < lower_bound) *it_r = lower_bound;
    *it_r *= 2500;
    it_r++;
    it_rpos++;
    it_rpos_next++;
  } 

  //////////////////////

  unsigned int L_chunk, L_chunk_plus_one;
  snp = 0;
  for(int chunk = 0; chunk < num_chunks; chunk++){

    FILE* fp_pos   = fopen(("chunk_" + std::to_string(chunk) + ".bp").c_str(), "wb");
    assert(fp_pos);
    FILE* fp_rpos  = fopen(("chunk_" + std::to_string(chunk) + ".rpos").c_str(), "wb");
    assert(fp_rpos);
    FILE* fp_r     = fopen(("chunk_" + std::to_string(chunk) + ".r").c_str(), "wb");
    assert(fp_r);
    //output chunk bed into fp_haps_chunk and chunk pos, rpos into fp_pos

    if(snp > 0){
      L_chunk = std::min(L-snp,chunk_size) + overlap; 
      L_chunk_plus_one = L_chunk + 1;
    }else{  
      L_chunk = std::min(L-snp,chunk_size);
      L_chunk_plus_one = L_chunk + 1; 
    }

    fwrite(&L_chunk,          sizeof(unsigned int), 1, fp_pos);
    fwrite(&L_chunk_plus_one, sizeof(unsigned int), 1, fp_rpos);
    fwrite(&L_chunk,          sizeof(unsigned int), 1, fp_r);
    fwrite(&pos[snp],         sizeof(int), L_chunk, fp_pos);
    fwrite(&rpos[snp],        sizeof(double), L_chunk + 1, fp_rpos);
    fwrite(&r[snp],           sizeof(double), L_chunk, fp_r);

    fclose(fp_pos);
    fclose(fp_rpos);
    fclose(fp_r);

    snp += std::min(L,(chunk+1)*chunk_size);

  }

} 

*/

void 
Data::MakeChunks(const std::string& filename_haps, const std::string& filename_sample, const std::string& filename_map, const std::string& filename_dist, float min_memory){


  haps m_haps(filename_haps.c_str(), filename_sample.c_str());
  N = m_haps.GetN();
  L = m_haps.GetL();
  std::vector<char>::size_type uN = N; 

  std::vector<int> bp_pos(L+1);
  std::vector<char> ancestral(L), alternative(L);
  std::vector<std::string> rsid(L);

  double min_memory_size = (min_memory) * 1e9/4.0 - (2*N*N + 3*N), actual_min_memory_size = 0.0; //5GB per window 
  if(min_memory_size <= 0){
    std::cerr << "Error: Need larger memory allowance." << std::endl;
    exit(1);
  }
  int windows_per_section = 500; //this is the number of open files, needs to be less than 500
  int max_windows_per_section = 0;

  int overlap = 20000;
  int max_chunk_size = min_memory_size/N;
  std::vector<std::vector<char> > p_seq(max_chunk_size), p_overlap(overlap);
  std::vector<std::vector<char> >::iterator it_p = p_seq.begin(), it_poverlap;
  for(; it_p != p_seq.end(); it_p++){
    (*it_p).resize(N);
  }

  int snp = 0;
  std::vector<int> window_boundaries(windows_per_section+1), window_boundaries_overlap(windows_per_section+1);
  std::vector<int>::iterator it_window_boundaries, it_window_overlap;
  std::vector<int> section_boundary_start, section_boundary_end;
  section_boundary_start.push_back(0);
  int min_snps_in_window = max_chunk_size;
  float mean_snps_in_window = 0.0;
  int num_windows, num_windows_overlap = 0;
  int overlap_in_section;
  int chunk_size;
  int chunk_index = 0;
  double window_memory_size = 0.0;
  while(snp < L){ 

    FILE* fp_haps_chunk = fopen(("chunk_" + std::to_string(chunk_index) + ".hap").c_str(), "wb");
    assert(fp_haps_chunk);
    //output chunk bed into fp_haps_chunk and chunk pos, rpos into fp_pos

    if(snp > 0){
     
      //copy the last #overlap snps;  
      assert(snp - section_boundary_start[section_boundary_start.size()-1] >= overlap);
      overlap_in_section = overlap;
      assert(overlap_in_section <= chunk_size);
      int snp_section_begin = snp - overlap_in_section;
      section_boundary_start.push_back(snp_section_begin);
      it_poverlap = p_overlap.begin();
      for(it_p = std::next(p_seq.begin(), chunk_size - overlap_in_section); it_p != std::next(p_seq.begin(), chunk_size); ){
        *it_poverlap = *it_p;
        it_poverlap++;
        it_p++;
      } 

      it_window_overlap = window_boundaries_overlap.begin();
      *it_window_overlap = snp_section_begin;
      it_window_overlap++;
      num_windows_overlap = 1;
      for(it_window_boundaries = window_boundaries.begin(); it_window_boundaries != std::next(window_boundaries.begin(), num_windows); it_window_boundaries++){
        if(*it_window_boundaries > snp_section_begin){
          *it_window_overlap = *it_window_boundaries;
          it_window_overlap++;
          num_windows_overlap++;
        }
      }
      assert(num_windows_overlap < windows_per_section-1);

    }


    int snp_begin             = snp;
    it_p                      = p_seq.begin();
    window_memory_size        = 0.0;
    chunk_size                = 0;
    
    window_boundaries[0] = snp_begin; 
    num_windows          = 1;
    int snps_in_window   = 0;
    while(num_windows + num_windows_overlap < windows_per_section && chunk_size < max_chunk_size && snp < L){
     
      m_haps.ReadSNP(*it_p, bp_pos[snp]);

      ancestral[snp]   = m_haps.ancestral;
      alternative[snp] = m_haps.alternative;
      rsid[snp]        = m_haps.rsid;

      int num_derived = 0;
      for(std::vector<char>::iterator it_snp = (*it_p).begin(); it_snp != (*it_p).end(); it_snp++){
        if(*it_snp == '1') num_derived++;
      }

      window_memory_size += num_derived * (N+1); //+ 2.88*N; //2.88 = 72/25 (72 bytes per 2 nodes, 1 tree in 25 snps) 
      //73 comes from  ((N+1+2*Node)*x + N^2 + 3*N) which I am using as an approximation of memory usage
      //I am also assuming one tree in 100 SNPs on average
      if(window_memory_size >= min_memory_size && snps_in_window > 10){
        if(actual_min_memory_size < window_memory_size) actual_min_memory_size = window_memory_size; 
        if(min_snps_in_window > snps_in_window) min_snps_in_window = snps_in_window;
        snps_in_window     = 0;
        window_memory_size = 0.0;
        window_boundaries[num_windows] = snp;
        num_windows++;
      }

      it_p++;
      snp++;
      snps_in_window++;
      chunk_size++;
    }
    if(actual_min_memory_size < window_memory_size) actual_min_memory_size = window_memory_size; 
    if(min_snps_in_window > snps_in_window) min_snps_in_window = snps_in_window;
    mean_snps_in_window = chunk_size/num_windows;
    window_boundaries[num_windows] = snp;
    assert(num_windows <= windows_per_section);

    if(num_windows > max_windows_per_section) max_windows_per_section = num_windows;

    //std::cerr << min_snps_in_window << " " << mean_snps_in_window << std::endl;
    if(mean_snps_in_window < 400){
      std::cerr << "Memory allowance should be set " << 400/mean_snps_in_window << " times larger than" << std::endl;
      std::cerr << "the current setting using --min_memory (Default 5GB)." << std::endl;
    }
    //std::cerr << snp << " " << chunk_size << " " << L << " " << num_windows << std::endl;

    section_boundary_end.push_back(snp);

    if(snp_begin == 0){
    
      std::vector<char>::size_type uL_chunk = chunk_size; 
      fwrite(&uL_chunk, sizeof(std::vector<char>::size_type), 1, fp_haps_chunk);
      fwrite(&uN, sizeof(std::vector<char>::size_type), 1, fp_haps_chunk);

      /////////////////// Output program parameters into file ///////////////
      FILE* fp = fopen(("parameters_c" + std::to_string(chunk_index) + ".bin").c_str(), "w");
      int num_windows_in_section = num_windows + 1;
      assert(fp);
      fwrite(&N, sizeof(int), 1, fp);
      fwrite(&chunk_size, sizeof(int), 1, fp);
      fwrite(&num_windows_in_section, sizeof(int), 1, fp);
      fwrite(&window_boundaries[0], sizeof(int), num_windows_in_section, fp);
      fclose(fp);
    
    }else{
   
      int L_chunk                           = chunk_size + overlap_in_section; 
      std::vector<char>::size_type uL_chunk = chunk_size + overlap_in_section; 
      fwrite(&uL_chunk, sizeof(std::vector<char>::size_type), 1, fp_haps_chunk);
      fwrite(&uN, sizeof(std::vector<char>::size_type), 1, fp_haps_chunk);

      int window_start = window_boundaries_overlap[0];
      for(it_window_boundaries = window_boundaries_overlap.begin(); it_window_boundaries != std::next(window_boundaries_overlap.begin(), num_windows_overlap); it_window_boundaries++){
        *it_window_boundaries -= window_start;
      }
      for(it_window_boundaries = window_boundaries.begin(); it_window_boundaries != std::next(window_boundaries.begin(), num_windows + 1); it_window_boundaries++){
        *it_window_boundaries -= window_start;
      }

      //std::cerr << window_boundaries_overlap[0] << " " << window_boundaries[0] << " " << num_windows_overlap << std::endl;

      /////////////////// Output program parameters into file ///////////////
      FILE* fp = fopen(("parameters_c" + std::to_string(chunk_index) + ".bin").c_str(), "w");
      int num_windows_in_section = num_windows + num_windows_overlap + 1;
      assert(fp);
      fwrite(&N, sizeof(int), 1, fp);
      fwrite(&L_chunk, sizeof(int), 1, fp);
      fwrite(&num_windows_in_section, sizeof(int), 1, fp);
      fwrite(&window_boundaries_overlap[0], sizeof(int), num_windows_overlap, fp);
      fwrite(&window_boundaries[0], sizeof(int), num_windows + 1, fp);
      fclose(fp);

      for(it_window_boundaries = window_boundaries.begin(); it_window_boundaries != std::next(window_boundaries.begin(), num_windows + 1); it_window_boundaries++){
        *it_window_boundaries += window_start;
      }

      for(it_p = p_overlap.begin(); it_p != std::next(p_overlap.begin(), overlap_in_section); it_p++){
        for(std::vector<char>::iterator it_row = (*it_p).begin(); it_row != (*it_p).end(); it_row++){ 
          fwrite(&(*it_row), sizeof(char), 1, fp_haps_chunk);
        }
      }
  
    }

    for(it_p = p_seq.begin(); it_p != std::next(p_seq.begin(), chunk_size); it_p++){
      for(std::vector<char>::iterator it_row = (*it_p).begin(); it_row != (*it_p).end(); it_row++){ 
        fwrite(&(*it_row), sizeof(char), 1, fp_haps_chunk);
      }
    }

    fclose(fp_haps_chunk);
    chunk_index++;

  }
  bp_pos[bp_pos.size()-1] = bp_pos[bp_pos.size()-2] + 1;
  m_haps.CloseFile();

  assert(section_boundary_end[section_boundary_end.size()-1] == L);
  assert(section_boundary_start.size() == section_boundary_end.size());
  int num_chunks = section_boundary_start.size();

  std::cerr << std::setprecision(2) << "Warning: Will use approx " << 2.0*(4.0 * N * N * (max_windows_per_section+2.0))/1e9 << "GB of hard disc." << std::endl;
  //std::cerr << "Warning: Will open max " << max_windows_per_section << " files." << std::endl;

  /////////////////// Output program parameters into file ///////////////
  FILE* fp = fopen("parameters.bin", "w");
  actual_min_memory_size += (2*N*N + 3*N);
  actual_min_memory_size *= 4.0/1e9;
  assert(fp);
  fwrite(&N, sizeof(int), 1, fp);
  fwrite(&L, sizeof(int), 1, fp);
  fwrite(&num_chunks, sizeof(int), 1, fp);
  fwrite(&actual_min_memory_size, sizeof(double), 1, fp);
  fwrite(&section_boundary_start[0], sizeof(int), num_chunks, fp);
  fwrite(&section_boundary_end[0], sizeof(int), num_chunks, fp);
  fclose(fp);

  //////////////////////// calculate rpos, r, pos here
  pos.resize(L); 
  std::vector<int>::iterator it_pos, it_bppos, it_bppos_next;

  if(filename_dist == "unspecified"){

    it_pos = pos.begin();
    it_bppos = bp_pos.begin();
    it_bppos_next = std::next(bp_pos.begin(), 1);
    int count = 0;
    for(; it_pos != std::prev(pos.end(),1);){
      *it_pos = *it_bppos_next - *it_bppos;
      if(*it_pos <= 0){
        std::cerr << "SNPs are not sorted by bp or more than one SNP at same position." << std::endl;
        exit(1);
      }
      it_pos++;
      it_bppos++;
      it_bppos_next++;
      count++;
    }
    pos[pos.size()-1] = 1;

  }else{

     
    char buffer[40];
    FILE* fp_dist = fopen(filename_dist.c_str(), "r");
    fscanf(fp_dist, "%s %s", buffer, buffer);
    int bp_dist, dist;
    snp = 0;
    while(fscanf(fp_dist, "%d %d", &bp_dist, &dist) !=EOF){
      assert(snp < L);
      assert(bp_pos[snp] == bp_dist);
      pos[snp] = dist;
      snp++;
    }
    fclose(fp_dist);
  
  }

  //output props
  FILE* fp_props = fopen("props.bin", "wb"); //storing rsid and other info in this file
  assert(fp_props);

  for(snp = 0; snp < L; snp++){
    fwrite(&snp, sizeof(int), 1, fp_props);
    fwrite(&bp_pos[snp], sizeof(int), 1, fp_props);
    fwrite(&pos[snp], sizeof(int), 1, fp_props);   
    fwrite(&rsid[snp][0], sizeof(char), 30, fp_props);
    fwrite(&ancestral[snp], sizeof(char), 1, fp_props);
    fwrite(&alternative[snp], sizeof(char), 1, fp_props);
  }

  fclose(fp_props);

  //parse map
  map m_map(filename_map.c_str());
  r.resize(L);
  rpos.resize(L+1); 
  std::vector<double>::iterator it_r, it_rpos;
  it_rpos = rpos.begin();
  it_bppos = bp_pos.begin();
  int map_pos = 0;

  if(m_map.bp[map_pos] > *it_bppos){
    *it_rpos = m_map.gen_pos[map_pos] * 1e-2;
    it_rpos++;
    it_bppos++;
  }
  for(; it_rpos != rpos.end();){
    while(m_map.bp[map_pos+1] <= *it_bppos && map_pos < m_map.bp.size()-2) map_pos++;
    if(m_map.bp[map_pos+1] < m_map.bp[map_pos]) std::cerr << m_map.bp[map_pos] << " " << m_map.bp[map_pos+1] << std::endl;
    assert(m_map.bp[map_pos+1] - m_map.bp[map_pos] >= 0);

    if(m_map.bp[map_pos+1] - m_map.bp[map_pos] == 0 || m_map.bp[map_pos] > *it_bppos){
      *it_rpos = m_map.gen_pos[map_pos] * 1e-2;
    }else{
      *it_rpos = ((*it_bppos - m_map.bp[map_pos])/((double)(m_map.bp[map_pos+1] - m_map.bp[map_pos])) * (m_map.gen_pos[map_pos+1] - m_map.gen_pos[map_pos]) +  m_map.gen_pos[map_pos]) * 1e-2;
    }

    it_rpos++;
    it_bppos++;
  }

  it_r = r.begin();
  it_rpos = rpos.begin();
  std::vector<double>::iterator it_rpos_next = std::next(rpos.begin(),1);
  for(; it_r != r.end();){
    *it_r = *it_rpos_next - *it_rpos;
    if(*it_r < lower_bound) *it_r = lower_bound;
    *it_r *= 2500;
    it_r++;
    it_rpos++;
    it_rpos_next++;
  } 

  //////////////////////

  unsigned int L_chunk, L_chunk_plus_one;
  for(int chunk = 0; chunk < num_chunks; chunk++){

    FILE* fp_pos   = fopen(("chunk_" + std::to_string(chunk) + ".bp").c_str(), "wb");
    assert(fp_pos);
    FILE* fp_rpos  = fopen(("chunk_" + std::to_string(chunk) + ".rpos").c_str(), "wb");
    assert(fp_rpos);
    FILE* fp_r     = fopen(("chunk_" + std::to_string(chunk) + ".r").c_str(), "wb");
    assert(fp_r);
    //output chunk bed into fp_haps_chunk and chunk pos, rpos into fp_pos

    L_chunk = section_boundary_end[chunk] - section_boundary_start[chunk]; 
    L_chunk_plus_one = L_chunk + 1;

    fwrite(&L_chunk,          sizeof(unsigned int), 1, fp_pos);
    fwrite(&L_chunk_plus_one, sizeof(unsigned int), 1, fp_rpos);
    fwrite(&L_chunk,          sizeof(unsigned int), 1, fp_r);
    fwrite(&pos[section_boundary_start[chunk]], sizeof(int), L_chunk, fp_pos);
    fwrite(&rpos[section_boundary_start[chunk]],sizeof(double), L_chunk + 1, fp_rpos);
    fwrite(&r[section_boundary_start[chunk]],   sizeof(double), L_chunk, fp_r);

    fclose(fp_pos);
    fclose(fp_rpos);
    fclose(fp_r);

  }

} 


/////////////////////////////

void 
Data::WriteSequenceAsBin(const char* filename){
  FILE* pf = fopen(filename, "wb");
  assert(pf != NULL);
  sequence.DumpToFile(pf);
  fclose(pf);
}

void 
Data::ReadSequenceFromBin(const char* filename){

  FILE* pf = fopen(filename, "rb");
  assert(pf != NULL);
  sequence.ReadFromFile(pf);
  L = sequence.size();
  N = sequence.subVectorSize(0);
  fclose(pf);
}

////////////////////////////
void
haps::ReadSNP(std::vector<char>& sequence, int& bp){

  //read a line, extract bp and fp_props
  //snp;pos_of_snp;rs-id;ancestral_allele/alternative_allele;downstream_allele;upstream_allele;All;
  fscanf(fp, "%d %s %d %c %c", &chr, rsid, &bp, &ancestral, &alternative);

  assert(sequence.size() > 0);
  //read haplotypes into sequence
  std::vector<char>::iterator it_seq = sequence.begin(); 
  int d;
  fgetc(fp);
  d = fgetc(fp);
  while(d != '\n' && d != EOF){
    if(d == '0'){
      *it_seq = '0';
    }else{
      *it_seq = '1';
    }
    d = fgetc(fp);
    if(d == '\n' || d == EOF) break;
    d = fgetc(fp);
    it_seq++;
  }

}

void
haps::DumpSNP(std::vector<char>& sequence, int bp, FILE* fp_out){

  //read a line, extract bp and fp_props
  //snp;pos_of_snp;rs-id;ancestral_allele/alternative_allele;downstream_allele;upstream_allele;All;
  fprintf(fp_out, "%d %s %d %c %c", chr, rsid, bp, ancestral, alternative);

  //read haplotypes into sequence
  std::vector<char>::iterator it_seq = sequence.begin(); 
  for(; it_seq != sequence.end(); it_seq++){
    fprintf(fp_out, " %c", *it_seq);
  }
  fprintf(fp_out, "\n");
  
}

map::map(const char* filename){

  fp = fopen(filename, "r");
  assert(fp);
  int lines = 0;
  while(!feof(fp)){
    if(fgetc(fp) == '\n'){
      lines++;
    }
  }
  lines--;//don't count the header
  fclose(fp);

  fp = fopen(filename, "r");
  assert(fp);
  //skip header
  fscanf(fp, "%s", buffer); 
  fscanf(fp, "%s", buffer); 
  fscanf(fp, "%s", buffer); 

  bp.resize(lines);
  gen_pos.resize(lines);

  float dummy;
  double fbp;
  for(int snp = 0; snp < lines; snp++){
    fscanf(fp, "%lf %f %lf", &fbp, &dummy, &gen_pos[snp]);
    bp[snp] = fbp;
  }

  fclose(fp);

}

void
fasta::Read(const std::string filename){

  std::ifstream is(filename);
  if(is.fail()){
    std::cerr << "Error while opening file " << filename << "." << std::endl;
    exit(1);
  }
  std::string line;
  getline(is,line);
  while(getline(is,line)){
    seq += line;
  }
  is.close();

}


