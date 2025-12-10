# Deployment Guide – Ordonnanceur Linux

## Table of Contents

1. [Deployment Overview](#deployment-overview)
2. [Local Development Setup](#local-development-setup)
3. [Production Builds](#production-builds)
4. [Deployment Targets](#deployment-targets)
5. [Docker Deployment](#docker-deployment)
6. [Cloud Deployment](#cloud-deployment)
7. [Performance Tuning](#performance-tuning)
8. [Monitoring & Logging](#monitoring--logging)
9. [Backup & Recovery](#backup--recovery)
10. [Rollback Procedures](#rollback-procedures)

---

## Deployment Overview

### Deployment Topology

```
┌─────────────────────────────────────────────────────┐
│          Development Environment                    │
│  - pnpm dev (port 3000)                             │
│  - Hot module reloading                             │
│  - Source maps enabled                              │
│  - Mock C backend optional                          │
└─────────────────────────────────────────────────────┘
                        │
                        ▼
┌─────────────────────────────────────────────────────┐
│          Staging Environment                        │
│  - pnpm build + pnpm start                          │
│  - Performance baseline                             │
│  - Security testing                                 │
│  - Load testing (optional)                          │
└─────────────────────────────────────────────────────┘
                        │
                        ▼
┌─────────────────────────────────────────────────────┐
│          Production Environment                     │
│  - Docker or native binary                          │
│  - Reverse proxy (nginx)                            │
│  - SSL/TLS enabled                                  │
│  - Monitoring & logging                             │
│  - Auto-scaling (cloud)                             │
└─────────────────────────────────────────────────────┘
```

### Deployment Checklist

**Pre-Deployment:**
- [ ] All tests passing (`make test`, `pnpm lint`)
- [ ] Version bumped in `package.json`
- [ ] CHANGELOG updated
- [ ] Performance benchmarks run
- [ ] Security audit passed
- [ ] Documentation updated

**Deployment:**
- [ ] Code reviewed and merged to `main` branch
- [ ] CI/CD pipeline triggered
- [ ] Build artifacts generated
- [ ] Deployed to staging first
- [ ] Staging tests pass
- [ ] Deployed to production
- [ ] Production health checks pass

**Post-Deployment:**
- [ ] Monitor error rates
- [ ] Check performance metrics
- [ ] Verify all features working
- [ ] Document deployment in release notes

---

## Local Development Setup

### Initial Setup

```bash
# Clone repository
git clone https://github.com/arijsebai/Projet-Ordonnancement-Linux.git
cd Projet-Ordonnancement-Linux
git checkout dev

# Install dependencies
pnpm install

# Build C backend
make clean && make all

# Verify build
ls -la ordonnanceur
```

### Development Server

```bash
# Start with hot reloading
pnpm dev

# Server output:
# > next dev
# ▲ Next.js 16.0.3
# - Local: http://localhost:3000

# Open browser to http://localhost:3000
```

### Environment Variables

Create `.env.local` for development:

```env
# Optional: API endpoint override
NEXT_PUBLIC_API_BASE=http://localhost:3000

# Optional: Analytics
NEXT_PUBLIC_VERCEL_ANALYTICS_ID=
NEXT_PUBLIC_ANALYTICS_ENABLED=false

# Optional: Debug mode
DEBUG=ordonnanceur:*
```

### Development Workflow

```bash
# 1. Make code changes
# 2. Frontend auto-reloads (HMR)
# 3. Changes to C backend require rebuild:
make clean && make all

# 4. Restart dev server to pick up new binary
# Ctrl+C, then pnpm dev
```

---

## Production Builds

### Build Process

```bash
# 1. Clean previous builds
make clean
pnpm clean

# 2. Compile C backend
make all
# Output: ./ordonnanceur (binary)

# 3. Build Next.js application
pnpm build
# Output: .next/ (optimized bundle)

# 4. Verify build artifacts
ls -la ordonnanceur
ls -la .next/
```

### Build Output

**Frontend (.next/ directory):**
```
.next/
├── app-build-manifest.json
├── build-manifest.json
├── required-server-files.json
├── standalone/              # Standalone mode (optional)
├── static/                  # CSS, JS chunks
│   ├── chunks/
│   ├── css/
│   └── media/
└── server/                  # Server-side code
```

**Backend (binary):**
```
ordonnanceur               # Executable binary (~1-5 MB)
```

### Production Next.js Configuration

**next.config.mjs:**
```javascript
export default {
  // Compression
  compress: true,
  
  // Remove Next.js header
  poweredByHeader: false,
  
  // Disable source maps in production
  productionBrowserSourceMaps: false,
  
  // SWC minification (faster than Terser)
  swcMinify: true,
  
  // Static optimization
  experimental: {
    optimizePackageImports: ["recharts", "@radix-ui/react-*"],
    staticGenerationRetryCount: 3,
  },
  
  // Environment variables
  env: {
    BUILD_TIME: new Date().toISOString(),
  },
};
```

### Optimization Flags

**C Backend Compilation:**
```makefile
# Add to Makefile for production builds
CFLAGS_PROD = -O3 -march=native -Wall -std=c11 -I$(INC_DIR)
# -O3: Maximum optimization
# -march=native: CPU-specific optimizations
```

```bash
# Compile with optimizations
CC=gcc CFLAGS="-O3 -march=native -Wall -std=c11" make all
```

---

## Deployment Targets

### Local Server (Development)

```bash
# Terminal 1: Build backend
make clean && make all

# Terminal 2: Start server
pnpm dev

# Access: http://localhost:3000
```

### Standalone Server (Production)

```bash
# 1. Build
pnpm build

# 2. Start server
pnpm start

# 3. Access: http://localhost:3000
# Note: Must have ordonnanceur binary in current directory
```

### Behind Nginx Reverse Proxy

**nginx.conf:**
```nginx
upstream next_app {
    server localhost:3000;
}

server {
    listen 80;
    server_name ordonnanceur.example.com;

    # Redirect HTTP to HTTPS
    return 301 https://$server_name$request_uri;
}

server {
    listen 443 ssl http2;
    server_name ordonnanceur.example.com;

    ssl_certificate /etc/letsencrypt/live/ordonnanceur.example.com/fullchain.pem;
    ssl_certificate_key /etc/letsencrypt/live/ordonnanceur.example.com/privkey.pem;

    # Security headers
    add_header Strict-Transport-Security "max-age=31536000; includeSubDomains" always;
    add_header X-Content-Type-Options "nosniff" always;
    add_header X-Frame-Options "SAMEORIGIN" always;
    add_header X-XSS-Protection "1; mode=block" always;

    # Compression
    gzip on;
    gzip_types text/plain text/css application/json application/javascript;
    gzip_min_length 1000;

    # Reverse proxy to Next.js
    location / {
        proxy_pass http://next_app;
        proxy_http_version 1.1;
        proxy_set_header Upgrade $http_upgrade;
        proxy_set_header Connection "upgrade";
        proxy_set_header Host $host;
        proxy_set_header X-Real-IP $remote_addr;
        proxy_set_header X-Forwarded-For $proxy_add_x_forwarded_for;
        proxy_set_header X-Forwarded-Proto $scheme;

        # Timeouts
        proxy_connect_timeout 60s;
        proxy_send_timeout 60s;
        proxy_read_timeout 60s;
    }

    # Static files caching
    location /_next/static {
        proxy_cache_valid 365d;
        proxy_pass http://next_app;
    }
}
```

**Start application:**
```bash
# Terminal 1: Start Next.js server on port 3000
pnpm start &

# Terminal 2: Start Nginx
sudo systemctl start nginx
# Access: https://ordonnanceur.example.com
```

---

## Docker Deployment

### Dockerfile (Multi-stage Build)

```dockerfile
# Stage 1: Builder
FROM node:18-alpine AS builder

WORKDIR /app

# Install pnpm
RUN npm install -g pnpm@8

# Copy dependency files
COPY package.json pnpm-lock.yaml ./

# Install Node dependencies
RUN pnpm install --frozen-lockfile

# Copy source code
COPY . .

# Install build tools (for C compilation)
RUN apk add --no-cache \
    build-base \
    gcc \
    make

# Build C backend
RUN make clean && make all

# Build Next.js frontend
RUN pnpm build

# Stage 2: Runtime
FROM node:18-alpine

WORKDIR /app

# Install runtime dependencies
RUN apk add --no-cache \
    gcc \
    musl-dev

# Copy package files
COPY package.json pnpm-lock.yaml ./

# Install production dependencies only
RUN npm install -g pnpm@8 && \
    pnpm install --frozen-lockfile --prod

# Copy build artifacts from builder
COPY --from=builder /app/.next ./.next
COPY --from=builder /app/ordonnanceur ./ordonnanceur
COPY --from=builder /app/config ./config
COPY --from=builder /app/public ./public

# Create non-root user
RUN addgroup -g 1001 -S nodejs && \
    adduser -S nextjs -u 1001

USER nextjs

# Health check
HEALTHCHECK --interval=30s --timeout=10s --start-period=5s --retries=3 \
    CMD node -e "require('http').get('http://localhost:3000', r => r.statusCode === 200 ? process.exit(0) : process.exit(1))"

EXPOSE 3000

ENV NODE_ENV=production

CMD ["pnpm", "start"]
```

### Docker Compose (Development)

```yaml
version: '3.9'

services:
  app:
    build: .
    ports:
      - "3000:3000"
    volumes:
      - ./config:/app/config
      - ./src:/app/src
      - ./app:/app/app
    environment:
      - NODE_ENV=development
    command: pnpm dev
```

### Build & Run

```bash
# Build image
docker build -t ordonnanceur:1.0.0 .

# Run container
docker run -p 3000:3000 \
  -v $(pwd)/config:/app/config \
  -e NODE_ENV=production \
  ordonnanceur:1.0.0

# Or use Docker Compose
docker-compose up
```

---

## Cloud Deployment

### Vercel (Recommended for Next.js)

**Benefits:**
- Serverless deployment
- Auto-scaling
- CDN edge locations
- Zero-config for Next.js
- Free tier available

**Setup:**

```bash
# 1. Install Vercel CLI
npm i -g vercel

# 2. Login
vercel login

# 3. Deploy
vercel

# 4. Configure build settings:
# Build Command: make clean && make all && pnpm build
# Output Directory: .next
# Environment: NODE_ENV=production
```

**vercel.json:**
```json
{
  "buildCommand": "make clean && make all && pnpm build",
  "outputDirectory": ".next",
  "env": {
    "NODE_ENV": "production"
  },
  "functions": {
    "api/**/*.ts": {
      "memory": 512,
      "maxDuration": 30
    }
  }
}
```

### AWS (EC2 + ELB)

**Architecture:**
```
Internet Gateway
    ↓
Elastic Load Balancer (HTTPS)
    ↓
EC2 Instances (Auto Scaling Group)
    ├─ Instance 1: pnpm start
    ├─ Instance 2: pnpm start
    └─ Instance 3: pnpm start
```

**CloudFormation Template (simplified):**
```yaml
Resources:
  LaunchTemplate:
    Type: AWS::EC2::LaunchTemplate
    Properties:
      LaunchTemplateData:
        ImageId: ami-0c55b159cbfafe1f0  # Ubuntu 22.04
        InstanceType: t3.medium
        UserData:
          Fn::Base64: |
            #!/bin/bash
            apt update && apt install -y \
              nodejs npm git build-essential gcc make
            npm install -g pnpm@8
            cd /home/ubuntu
            git clone https://github.com/arijsebai/Projet-Ordonnancement-Linux.git
            cd Projet-Ordonnancement-Linux
            git checkout main
            pnpm install
            make clean && make all
            pnpm build
            pnpm start &

  AutoScalingGroup:
    Type: AWS::AutoScaling::AutoScalingGroup
    Properties:
      LaunchTemplate:
        LaunchTemplateId: !Ref LaunchTemplate
        Version: !GetAtt LaunchTemplate.LatestVersionNumber
      MinSize: 2
      MaxSize: 10
      DesiredCapacity: 3
      TargetGroupARNs:
        - !Ref ALBTargetGroup
      VPCZoneIdentifier: !Ref PrivateSubnets

  LoadBalancer:
    Type: AWS::ElasticLoadBalancingV2::LoadBalancer
    Properties:
      Type: application
      Scheme: internet-facing
      Subnets: !Ref PublicSubnets
      SecurityGroups:
        - !Ref ALBSecurityGroup

  Listener:
    Type: AWS::ElasticLoadBalancingV2::Listener
    Properties:
      LoadBalancerArn: !Ref LoadBalancer
      Port: 443
      Protocol: HTTPS
      Certificates:
        - CertificateArn: !Ref SSLCertificate
      DefaultActions:
        - Type: forward
          TargetGroupArn: !Ref ALBTargetGroup
```

### DigitalOcean (VPS)

```bash
# 1. Create Droplet (Ubuntu 22.04, 2GB RAM)
# 2. SSH into droplet
ssh root@<IP>

# 3. Setup system
apt update && apt upgrade -y
apt install -y nodejs npm git build-essential gcc make

# 4. Install pnpm & clone repo
npm install -g pnpm@8
git clone https://github.com/arijsebai/Projet-Ordonnancement-Linux.git
cd Projet-Ordonnancement-Linux

# 5. Build
pnpm install
make clean && make all
pnpm build

# 6. Create systemd service
cat > /etc/systemd/system/ordonnanceur.service <<EOF
[Unit]
Description=Ordonnanceur Linux Scheduler
After=network.target

[Service]
Type=simple
User=www-data
WorkingDirectory=/root/Projet-Ordonnancement-Linux
ExecStart=/usr/bin/npm run start
Restart=on-failure
RestartSec=10

[Install]
WantedBy=multi-user.target
EOF

# 7. Start service
systemctl daemon-reload
systemctl enable ordonnanceur
systemctl start ordonnanceur

# 8. Setup Nginx reverse proxy (as shown earlier)
```

---

## Performance Tuning

### Frontend Optimization

**Code Splitting:**
```typescript
// Use dynamic imports for large components
import dynamic from 'next/dynamic'

const ResultsDisplay = dynamic(
  () => import('@/components/results-display'),
  { loading: () => <Skeleton /> }
)
```

**Image Optimization:**
```typescript
import Image from 'next/image'

export default function Logo() {
  return (
    <Image
      src="/logo.png"
      alt="Logo"
      width={200}
      height={50}
      priority
    />
  )
}
```

**Bundle Analysis:**
```bash
# Analyze bundle size
npm install --save-dev @next/bundle-analyzer

# In next.config.mjs:
import withBundleAnalyzer from '@next/bundle-analyzer'

export default withBundleAnalyzer({
  enabled: process.env.ANALYZE === 'true',
})(config)

# Run analysis
ANALYZE=true pnpm build
```

### Backend Optimization

**Compiler Flags:**
```makefile
# In Makefile
ifeq ($(MODE), release)
    CFLAGS += -O3 -march=native -ffast-math
else
    CFLAGS += -g -O0  # Debug mode
endif

# Build with: MODE=release make
```

**Algorithm Optimization:**
```c
// Cache frequently accessed values
int n_ready = 0;
for (int i = 0; i < n; i++) {
    if (procs[i].remaining_time > 0 && procs[i].arrival_time <= time) {
        n_ready++;
        // Use cached value
    }
}
```

**Profiling:**
```bash
# Compile with profiling symbols
gcc -pg -O2 -o ordonnanceur_prof src/*.c policies/*.c

# Run with test data
./ordonnanceur_prof config/sample_config.txt

# Analyze
gprof ordonnanceur_prof gmon.out
```

### Memory Optimization

**Reduce Memory Usage:**
```c
// Use bitfields for boolean values
struct process {
    char name[16];
    int arrival_time;
    int exec_time;
    int priority;
    unsigned int is_completed : 1;     // 1 bit instead of 4 bytes
    unsigned int is_waiting : 1;       // 1 bit instead of 4 bytes
};

// Use smaller data types where possible
// uint8_t instead of int for values 0-255
// int16_t instead of int for values -32k to +32k
```

---

## Monitoring & Logging

### Application Monitoring

**Frontend Error Tracking (Optional):**
```typescript
// app/layout.tsx
import * as Sentry from "@sentry/nextjs"

Sentry.init({
  dsn: process.env.NEXT_PUBLIC_SENTRY_DSN,
  tracesSampleRate: 0.1,
  environment: process.env.NODE_ENV,
})
```

**Custom Logging:**
```typescript
// lib/logger.ts
export const logger = {
  info: (msg: string, data?: any) => {
    console.log(`[INFO] ${new Date().toISOString()} ${msg}`, data)
  },
  error: (msg: string, err?: Error) => {
    console.error(`[ERROR] ${new Date().toISOString()} ${msg}`, err)
  },
}
```

### Backend Logging

```c
// include/logger.h
#define LOG_INFO(fmt, ...) \
    fprintf(stderr, "[INFO] " fmt "\n", ##__VA_ARGS__)

#define LOG_ERROR(fmt, ...) \
    fprintf(stderr, "[ERROR] " fmt "\n", ##__VA_ARGS__)

// Usage
LOG_INFO("Loaded %d processes", n);
LOG_ERROR("Failed to parse config: %s", path);
```

### Metrics Collection

**Performance Metrics:**
```typescript
// Track API response times
export async function POST(req: Request) {
  const startTime = performance.now()
  
  // ... process request ...
  
  const duration = performance.now() - startTime
  console.log(`Schedule API took ${duration.toFixed(2)}ms`)
  
  return response
}
```

**System Metrics:**
```bash
# Monitor using system tools
# CPU usage, memory, disk I/O
top -p $(pgrep -f 'pnpm start')

# Check disk space
df -h

# Monitor logs
tail -f /var/log/ordonnanceur.log
```

---

## Backup & Recovery

### Data Backup

**Configuration Files:**
```bash
# Backup config directory
tar -czf config_backup_$(date +%Y%m%d).tar.gz config/

# Store in safe location
cp config_backup_$(date +%Y%m%d).tar.gz /backup/

# Restore
tar -xzf config_backup_20240101.tar.gz
```

**Application Code:**
```bash
# Already in git, but can create snapshot
git tag -a v1.0.0-prod-snapshot -m "Production snapshot"
git push origin v1.0.0-prod-snapshot

# Create full backup
tar -czf ordonnanceur_full_$(date +%Y%m%d).tar.gz \
  --exclude=node_modules \
  --exclude=.next \
  --exclude=build \
  .
```

### Database Backups (if applicable)

```bash
# If using any database
pg_dump ordonnanceur > ordonnanceur_db_$(date +%Y%m%d).sql

# Compress and archive
gzip ordonnanceur_db_$(date +%Y%m%d).sql
```

### Automated Backups

**Cron Job:**
```bash
# Add to crontab (crontab -e)
# Daily backup at 2 AM
0 2 * * * /usr/local/bin/backup_ordonnanceur.sh

# Create backup script
cat > /usr/local/bin/backup_ordonnanceur.sh <<'EOF'
#!/bin/bash
BACKUP_DIR="/backups/ordonnanceur"
DATE=$(date +%Y%m%d_%H%M%S)
mkdir -p $BACKUP_DIR

# Backup config
tar -czf $BACKUP_DIR/config_$DATE.tar.gz /root/Projet-Ordonnancement-Linux/config/

# Keep only last 7 days
find $BACKUP_DIR -name "config_*.tar.gz" -mtime +7 -delete
EOF

chmod +x /usr/local/bin/backup_ordonnanceur.sh
```

---

## Rollback Procedures

### Rollback Steps

**1. Identify Issue:**
```bash
# Check recent logs
tail -100 /var/log/ordonnanceur.log | grep ERROR

# Check health
curl -i http://localhost:3000/
```

**2. Rollback to Previous Version:**
```bash
# Option A: Git-based rollback
git checkout <previous-commit-hash>

# Option B: Docker rollback
docker run -p 3000:3000 ordonnanceur:1.0.0-previous

# Option C: Systemd service rollback
systemctl stop ordonnanceur
cd /root/Projet-Ordonnancement-Linux
git checkout v1.0.0
pnpm install
make clean && make all
pnpm build
systemctl start ordonnanceur
```

**3. Verify:**
```bash
# Check if service is running
systemctl status ordonnanceur

# Test functionality
curl http://localhost:3000/api/parse-config \
  -H "Content-Type: application/json" \
  -d '{"defaultFile":"sample_config.txt"}'

# Monitor logs
tail -f /var/log/ordonnanceur.log
```

**4. Notify Team:**
```bash
# Log incident
echo "Rollback from v1.0.1 to v1.0.0 due to [issue]" >> INCIDENTS.log
```

### Blue-Green Deployment (Zero Downtime)

**Setup:**
```bash
# Run two instances
pnpm start &  # Blue (port 3000)
pnpm start &  # Green (port 3001)

# Nginx routes to Blue
# When deploying, deploy to Green
# Test Green
# Switch Nginx to Green
# Green becomes new Blue
```

**Nginx Configuration:**
```nginx
upstream blue {
    server localhost:3000;
}

upstream green {
    server localhost:3001;
}

# Set active upstream
set $active_upstream blue;

server {
    location / {
        proxy_pass http://$active_upstream;
    }
}

# To switch: change $active_upstream to green
# Then: update Blue with new version
# Then: switch back when ready
```

---

## Post-Deployment Checklist

- [ ] All endpoints respond (200 OK)
- [ ] Error pages display correctly
- [ ] Static assets load (check Network tab)
- [ ] C backend binary exists and is executable
- [ ] Sample config loads on startup
- [ ] Can upload custom config files
- [ ] All 6 algorithms selectable
- [ ] Simulation executes without errors
- [ ] Results display correctly
- [ ] Charts render properly
- [ ] Mobile responsive (test on mobile device)
- [ ] Performance acceptable (< 3s for 100 processes)
- [ ] No console errors
- [ ] Logging working
- [ ] Monitoring alerts configured
- [ ] Backups running

---

**Last Updated:** December 2024  
**Version:** 1.0.0  
**Deployment Status:** Production Ready ✅

