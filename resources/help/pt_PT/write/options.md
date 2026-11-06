# Alterar as definições do JeffWrite
<!-- keywords: substituir ao escrever, tecla insert, escrever substitui seleção, ctrl+clique, seguir ligação, arrastar e largar, informações pessoais, remover informações pessoais, avançadas, unidades, medidas, polegadas, centímetros, opções, definições, preferências, guardar, formato predefinido, pasta predefinida, localização predefinida do ficheiro, recuperação, marcas de formatação, linhas de grelha, visualização, estatísticas de utilização, número de instalação, privacidade, aviso de privacidade, JP_NO_TELEMETRY, eliminar estatísticas, atualizações, acerca de -->

As suas definições estão na janela **Opções do JeffWrite**. Clique em **Ficheiro > Opções** para a abrir. Clique em **OK** para manter as alterações, ou em **Cancelar** para as descartar.

## Guardar

A página **Guardar** da janela Opções tem quatro opções.

- **Guardar novos documentos como** escolhe o tipo de ficheiro que **Guardar Como** propõe em primeiro lugar para um documento que ainda não tem ficheiro: um ficheiro .docx ou um ficheiro .odt.
- **Guardar informações de recuperação automática a cada** indica quantos minutos decorrem entre as cópias de segurança que o JeffWrite mantém do trabalho não guardado. Veja [Abrir, guardar, imprimir e exportar](open-save).
- **Localização predefinida do ficheiro** é a pasta em que as caixas **Abrir** e **Guardar Como** começam. Clique em **Procurar** para a escolher. Deixe o campo vazio para começar na pasta Documentos.
- **Remover informações pessoais das propriedades do ficheiro ao guardar** retira o seu nome do ficheiro sempre que o guarda. As propriedades do ficheiro incluem o autor, a pessoa que o guardou por último, a empresa e o gestor. O título e as outras propriedades mantêm-se. Os nomes também são retirados do documento aberto, pelo que continuam ausentes até o fechar. Para o fazer apenas uma vez, use **Inspecionar Documento** em [Informações](info).

## Visualização

A página **Visualização** escolhe o que uma janela mostra quando é aberta.

- **Mostrar marcas de formatação** ativa as marcas de parágrafo e outros sinais que não são impressos. Cada janela pode ainda ativar ou desativar estas marcas com o botão **¶**.
- **Mostrar linhas de grelha das tabelas** desenha as margens ténues das células das tabelas que não têm linhas.

Uma alteração nesta página aplica-se de imediato à janela em que está e a cada janela que abrir a seguir.

## Avançadas

**Mostrar medidas em** escolhe a unidade para a régua e as caixas de diálogo: polegadas, centímetros, milímetros, pontos ou paicas. Um ponto corresponde a 1/72 de polegada. Uma paica corresponde a 12 pontos.

**Utilizar Ctrl+clique para seguir uma ligação** está ativado por predefinição: mantém premida a tecla **Ctrl** e clica numa ligação para lá ir, e um clique simples apenas coloca o cursor no texto da ligação. Desative esta opção para seguir uma ligação com um clique simples. Manter premida a tecla **Shift** ao clicar continua a selecionar texto.

**Permitir arrastar e largar texto** permite selecionar palavras e arrastá-las para um novo local. Mantenha premida a tecla **Ctrl** ao largar para copiar em vez de mover. Desative esta opção se arrastar por engano.

**Escrever substitui o texto selecionado** está ativado por predefinição: quando há palavras selecionadas e escreve, as novas letras ocupam o seu lugar. Se desativar, as novas letras são colocadas depois das palavras selecionadas.

**Utilizar a tecla Insert para ativar e desativar o modo de substituição** está desativado por predefinição. Quando está ativo, a tecla **Insert** alterna entre duas formas de escrever. Normalmente, cada letra que escreve empurra as letras seguintes. No modo **substituição**, cada letra que escreve substitui a letra à direita do cursor. Prima **Insert** novamente para voltar ao modo normal.

Estas opções aplicam-se de imediato à janela em que está e a cada janela que abrir a seguir.

## Estatísticas de utilização

**Enviar estatísticas de utilização anónimas** permite partilhar dados sobre a forma como o JeffWrite é utilizado. Anónimo significa que os dados não revelam quem é. Quando esta opção está ativa, o JeffWrite envia estes dados uma vez por dia e no dia em que atualiza: que se trata do JeffWrite, a versão, o sistema operativo e o idioma, quantas vezes foi iniciado, com que frequência cada comando é utilizado e que tipo de ficheiro abriu ou guardou. Se o JeffWrite fechar inesperadamente, envia também um relatório de falha na próxima vez que for iniciado. O JeffWrite nunca envia os seus ficheiros, os nomes dos mesmos nem o respetivo conteúdo, nem o que escreveu. Se esta opção estiver desativada, também não é enviado nenhum relatório de falha.

A escolha aplica-se a todas as aplicações JeffOffice: se a alterar aqui, altera-se em cada uma delas. O JeffWrite pergunta por ela na primeira vez que é iniciado, numa janela com o nome **Ajude a melhorar o JeffWrite**.

- **Aviso de privacidade**, ao lado da caixa de verificação, abre o nosso aviso de privacidade em jeffoffice.com/privacy. Nele se indica o que é enviado, para onde vai e durante quanto tempo o guardamos. Conservamos as estatísticas durante 24 meses.
- **Número de instalação** é apresentado por baixo da caixa de verificação. É um número aleatório gerado para a sua cópia do JeffOffice, igual em todas as aplicações JeffOffice. Não revela nada sobre si. Clique em **Copiar** para o copiar. Se as estatísticas estiverem desativadas, ou se ainda não tiver sido gerado nenhum número, aparece **nenhum**.
- Para que as estatísticas que já temos sejam eliminadas, envie o seu número de instalação para info@jeffoffice.com **antes** de desativar as estatísticas. Ao desativá-las, o número é apagado do seu computador e, sem ele, não conseguimos identificar que registos são seus.
- Para desativar as estatísticas sem abrir o JeffWrite, defina a variável de ambiente `JP_NO_TELEMETRY` com qualquer valor antes de o iniciar. Assim, nada é contabilizado nem enviado, e o JeffWrite não faz a pergunta da primeira utilização.

## Consulte também

- [Bem-vindo ao JeffWrite](start)
