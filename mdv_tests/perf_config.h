#ifndef MDV_PERF_CONFIG_H
#define MDV_PERF_CONFIG_H

// Performance test configuration parameters
typedef struct {
    // Bulk operations
    int bulk_batch_size;        // Rows per bulk batch
    int bulk_total_inserts;     // Total bulk inserts
    int bulk_total_updates;     // Total bulk updates
    int bulk_total_reads;       // Total bulk reads
    
    // Single operations
    int single_total_inserts;   // Total single inserts
    int single_total_updates;   // Total single updates
    int single_total_reads;     // Total single reads
    int single_total_deletes;   // Total single deletes
    
    // Test configuration
    int warmup_iterations;      // Warmup before measurements
    int measurement_samples;    // Samples for averaging
} mdv_perf_config;

#define DEFAULT_PERF_CONFIG (mdv_perf_config){ \
   .bulk_batch_size = 5, \
   .bulk_total_inserts = 10, \
   .bulk_total_updates = 10, \
   .bulk_total_reads = 10, \
   .single_total_inserts = 10, \
   .single_total_updates = 10, \
   .single_total_reads = 10, \
   .single_total_deletes = 10, \
   .warmup_iterations = 5, \
   .measurement_samples = 3 \
}

#endif // MDV_PERF_CONFIG_H