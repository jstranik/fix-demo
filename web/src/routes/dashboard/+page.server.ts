import { env } from "$env/dynamic/private";
import type { Actions } from "@sveltejs/kit";
import type { PageServerLoad } from "./$types";
import { promises as fs } from "fs";

export const load = async function(e) {
    const c = await fs.readFile(env.LOGIC_FILE)
    return {
        logic: c.toString()
    }
} satisfies PageServerLoad


export const actions = {
    default: async(e) => {
        const data = await e.request.formData()
        const logic = data.get("logic")?.toString()
        if (logic)
            await fs.writeFile(env.LOGIC_FILE, logic)
        return {
            logic: logic
        }
    }
} satisfies Actions