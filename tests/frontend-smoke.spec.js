const { test, expect } = require('@playwright/test')
const fs = require('fs')
const path = require('path')

const BASE_URL = 'http://127.0.0.1:8080'
const LOGIN_URL = `${BASE_URL}/#/login`
const REPORT_PATH = path.join(process.cwd(), 'plans', 'frontend-vue-test-report.json')

const report = []

function createSessionResult(name) {
    const session = {
        account: name,
        startedAt: new Date().toISOString(),
        consoleErrors: [],
        pageErrors: [],
        requestFailures: [],
        apiErrors: [],
        checkpoints: []
    }
    report.push(session)
    return session
}

function attachObservers(page, session) {
    page.on('console', msg => {
        if (msg.type() === 'error') {
            session.consoleErrors.push(msg.text())
        }
    })

    page.on('pageerror', error => {
        session.pageErrors.push(String(error))
    })

    page.on('requestfailed', request => {
        session.requestFailures.push({
            url: request.url(),
            method: request.method(),
            failure: request.failure()?.errorText || 'unknown'
        })
    })

    page.on('response', async response => {
        const url = response.url()
        if (url.includes('/api/v1') && response.status() >= 400) {
            let body = ''
            try {
                body = await response.text()
            } catch (_err) { }
            session.apiErrors.push({
                url,
                status: response.status(),
                body: body.slice(0, 500)
            })
        }
    })
}

async function login(page, username, password) {
    await page.goto(LOGIN_URL, { waitUntil: 'domcontentloaded' })
    await page.getByPlaceholder('请输入管理员/医生/护士账号').fill(username)
    await page.getByPlaceholder('请输入密码').fill(password)
    await page.getByRole('button', { name: /登\s*录/ }).click()
}

async function waitForSettled(page) {
    await page.waitForLoadState('domcontentloaded')
    await page.waitForTimeout(1200)
}

async function expectVisible(page, selector, label, session) {
    const locator = page.locator(selector)
    await expect(locator, label).toBeVisible({ timeout: 10000 })
    session.checkpoints.push({ label, status: 'passed', url: page.url() })
}

async function navigateMenu(page, menuText, expectedUrlPattern, selectorToCheck, session) {
    await page.locator('.el-menu-item', { hasText: menuText }).click()
    await page.waitForURL(expectedUrlPattern, { timeout: 10000 })
    await waitForSettled(page)
    await expectVisible(page, selectorToCheck, `页面 ${menuText} 已正常渲染`, session)
}

test.afterAll(async () => {
    fs.mkdirSync(path.dirname(REPORT_PATH), { recursive: true })
    fs.writeFileSync(REPORT_PATH, JSON.stringify(report, null, 2), 'utf8')
})

test('admin 账号页面切换与管理员权限测试', async ({ page }) => {
    const session = createSessionResult('admin')
    attachObservers(page, session)

    await login(page, 'admin', '123456')
    await page.waitForURL(/#\/(dashboard)?$/, { timeout: 10000 }).catch(async () => {
        await page.waitForURL(/#\/dashboard/, { timeout: 10000 })
    })
    await waitForSettled(page)

    await expectVisible(page, 'text=快捷操作向导', '管理员登录后进入数据概览页', session)
    await expect(page.locator('.el-menu-item', { hasText: '系统管理' })).toBeVisible({ timeout: 10000 })
    session.checkpoints.push({ label: '管理员菜单可见', status: 'passed', url: page.url() })

    await navigateMenu(page, '居民管理', /#\/residents$/, 'input[placeholder="搜索居民姓名/身份证号"]', session)
    await navigateMenu(page, '健康档案', /#\/health$/, 'text=请输入居民姓名搜索', session)
    await navigateMenu(page, '随访管理', /#\/visits$/, 'text=请输入居民姓名搜索随访记录', session)
    await navigateMenu(page, '数据概览', /#\/dashboard$/, 'text=快捷操作向导', session)
    await navigateMenu(page, '系统管理', /#\/system\/users$/, 'button:has-text("新增系统账户")', session)

    await page.reload({ waitUntil: 'domcontentloaded' })
    await waitForSettled(page)
    await expect(page).toHaveURL(/#\/system\/users/)
    session.checkpoints.push({ label: '管理员页面刷新后仍停留在用户管理页', status: 'passed', url: page.url() })
})

test('doctor_zhang 账号页面切换与管理员拦截测试', async ({ page }) => {
    const session = createSessionResult('doctor_zhang')
    attachObservers(page, session)

    await login(page, 'doctor_zhang', '123456')
    await page.waitForURL(/#\/(dashboard)?$/, { timeout: 10000 }).catch(async () => {
        await page.waitForURL(/#\/dashboard/, { timeout: 10000 })
    })
    await waitForSettled(page)

    await expectVisible(page, 'text=快捷操作向导', '医生登录后进入数据概览页', session)
    await expect(page.locator('.el-menu-item', { hasText: '系统管理' })).toHaveCount(0)
    session.checkpoints.push({ label: '医生账号不显示系统管理菜单', status: 'passed', url: page.url() })

    await navigateMenu(page, '居民管理', /#\/residents$/, 'input[placeholder="搜索居民姓名/身份证号"]', session)
    await navigateMenu(page, '健康档案', /#\/health$/, 'text=请输入居民姓名搜索', session)
    await navigateMenu(page, '随访管理', /#\/visits$/, 'text=请输入居民姓名搜索随访记录', session)

    await page.goto(`${BASE_URL}/#/system/users`, { waitUntil: 'domcontentloaded' })
    await waitForSettled(page)
    await expect(page).not.toHaveURL(/#\/system\/users/)
    await expectVisible(page, 'text=快捷操作向导', '医生强行访问管理员路由后被拦截回首页', session)
})

test('nurse_wang 账号页面切换与管理员拦截测试', async ({ page }) => {
    const session = createSessionResult('nurse_wang')
    attachObservers(page, session)

    await login(page, 'nurse_wang', '123456')
    await page.waitForURL(/#\/(dashboard)?$/, { timeout: 10000 }).catch(async () => {
        await page.waitForURL(/#\/dashboard/, { timeout: 10000 })
    })
    await waitForSettled(page)

    await expectVisible(page, 'text=快捷操作向导', '护士登录后进入数据概览页', session)
    await expect(page.locator('.el-menu-item', { hasText: '系统管理' })).toHaveCount(0)
    session.checkpoints.push({ label: '护士账号不显示系统管理菜单', status: 'passed', url: page.url() })

    await navigateMenu(page, '居民管理', /#\/residents$/, 'input[placeholder="搜索居民姓名/身份证号"]', session)
    await navigateMenu(page, '健康档案', /#\/health$/, 'text=请输入居民姓名搜索', session)
    await navigateMenu(page, '随访管理', /#\/visits$/, 'text=请输入居民姓名搜索随访记录', session)

    await page.goto(`${BASE_URL}/#/system/users`, { waitUntil: 'domcontentloaded' })
    await waitForSettled(page)
    await expect(page).not.toHaveURL(/#\/system\/users/)
    await expectVisible(page, 'text=快捷操作向导', '护士强行访问管理员路由后被拦截回首页', session)
})
