<template>
  <el-card shadow="hover" :class="['stat-card', bgClass]" :body-style="{ display: 'flex', alignItems: 'center', padding: '24px 20px' }">
    <div class="stat-icon">
      <slot name="icon"></slot>
    </div>
    <div class="stat-content">
      <div class="stat-title">{{ title }}</div>
      <div class="stat-value">
        <slot name="value">{{ value }}</slot>
      </div>
    </div>
  </el-card>
</template>

<script setup lang="ts">
defineProps<{
  title: string
  value?: number | string
  bgClass: string
}>()
</script>

<style scoped>
.stat-card {
  color: #fff;
  border: none;
  margin-bottom: 20px;
  border-radius: 8px;
}

.stat-card.bg-blue { background: linear-gradient(135deg, #36D1DC 0%, #5B86E5 100%); }
.stat-card.bg-green { background: linear-gradient(135deg, #11998e 0%, #38ef7d 100%); }
.stat-card.bg-orange { background: linear-gradient(135deg, #ff9966 0%, #ff5e62 100%); }
.stat-card.bg-red { background: linear-gradient(135deg, #ed213a 0%, #93291e 100%); }

.stat-icon {
  font-size: 46px;
  opacity: 0.85;
  margin-right: 18px;
}

.stat-content {
  display: flex;
  flex-direction: column;
  justify-content: center;
}

.stat-title {
  font-size: 15px;
  font-weight: 500;
  opacity: 0.9;
  margin-bottom: 6px;
}

.stat-value {
  font-size: 28px;
  font-weight: bold;
  line-height: 1.2;
}
</style>
