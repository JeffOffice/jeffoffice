# Info: propriedades, senhas e Inspecionar

<!-- keywords: informações, propriedades, título, autor, assunto, palavras-chave, categorias, comentários, empresa, gerente, propriedades personalizadas, marcar como final, final, somente leitura, senha, criptografar, proteger, inspecionar pasta de trabalho, informações pessoais, planilhas ocultas, anotações, links externos, XML personalizado, mostrar na pasta, copiar caminho, pasta, caminho, histórico de versões, versões, voltar -->

Clique em **Arquivo > Informações** para ver os dados da sua pasta de trabalho e protegê-la antes de compartilhá-la. Pressione **Esc** para voltar à sua pasta de trabalho.

## Onde está o arquivo

No alto da página, aparece onde o arquivo está salvo. Clique em **Mostrar na Pasta** para abrir essa pasta. Clique em **Copiar Caminho** para copiar o caminho e colá-lo em um email ou em um chat.

## Propriedades

Propriedades são dados sobre a pasta de trabalho, como o seu título e quem a escreveu. A página tem uma caixa para cada uma: **Título**, **Autor**, **Assunto**, **Palavras-chave**, **Categorias**, **Comentários**, **Empresa** e **Gerente**. Digite em uma caixa e pressione **Tab** ou **Enter**. O JeffCrunch guarda a alteração e a grava no arquivo na próxima vez que você salvar a pasta de trabalho. A página também mostra quando a pasta de trabalho foi criada, salva e impressa.

Clique em **Propriedades Personalizadas** para criar propriedades próprias, como **Projeto** ou **Cliente**. Clique em **Adicionar**, digite um nome, escolha o tipo de valor (texto, número inteiro, número, sim ou não, ou data) e digite o valor. O JeffCrunch salva as propriedades do jeito que você digitou.

## Marcar como Final

Clique em **Marcar como Final** para avisar os leitores de que a pasta de trabalho está concluída. O JeffCrunch salva a pasta de trabalho e a mantém apenas para leitura. Quando alguém a abre, a barra de título indica que ela é somente leitura. Isso é uma forma gentil de dizer "por favor, não altere isto". Não é um bloqueio, porque qualquer pessoa pode escolher **Marcado como Final** novamente para editá-la.

## Sempre Abrir como Somente Leitura

Clique em **Sempre Abrir como Somente Leitura** para que a pasta de trabalho seja aberta apenas para leitura todas as vezes, e ninguém a altere por engano. A escolha é salva com o arquivo quando você o salva. Na próxima vez que alguém abrir o arquivo, a barra de status informa que ele está aberto apenas para leitura. Para editá-lo, abra **Arquivo > Informações** e clique em **Sempre Abre como Somente Leitura**. É preciso apenas um clique, então isso é um lembrete e não um bloqueio. Outros programas não leem essa escolha. Um arquivo de texto, um arquivo .xls e um arquivo .ods não podem armazená-la, então salve a pasta de trabalho como arquivo .xlsx antes.

## Criptografar com Senha

Clique em **Criptografar com Senha** para pedir uma senha sempre que o arquivo for aberto. É o mesmo que **Proteger com Senha**. Veja [Abrir pastas de trabalho protegidas por senha](open-password). Uma senha protege um arquivo **.xlsx** com proteção forte. Se você esquecer a senha, ela **não pode ser recuperada**. As versões que você guardou e a cópia de backup seguem a senha.

Para impedir alterações em uma planilha ou no seu layout, veja [Proteger uma planilha ou a pasta de trabalho](protect).

## Proteger a planilha ou as planilhas

**Proteger Planilha Atual** permite escolher o que as pessoas podem alterar na planilha em que você está, com uma senha, se quiser. **Proteger Estrutura da Pasta de Trabalho** impede que as pessoas adicionem, excluam, movam, ocultem ou renomeiem planilhas. Os dois também estão na guia **Revisão**.

## Histórico de Versões

**Histórico de Versões** guarda cópias da sua pasta de trabalho neste computador. Nada é enviado a lugar nenhum. Uma cópia se chama versão. Guarde uma antes de uma alteração grande.

- **Manter uma Versão** salva a pasta de trabalho do jeito que está agora, com um nome que você escolhe.
- **Abrir uma Cópia** abre uma versão em uma janela própria, para que você a veja ao lado da pasta de trabalho.
- **Voltar a Esta Versão** coloca uma versão de volta como sua pasta de trabalho. A pasta de trabalho atual é guardada como versão antes, então nada se perde.
- **Excluir** descarta uma versão.

A pasta de trabalho precisa estar salva antes, porque as versões pertencem a um arquivo. Voltar a uma versão funciona para um arquivo .xlsx.

## Inspecionar Pasta de Trabalho

Clique em **Inspecionar Pasta de Trabalho** antes de enviar uma pasta de trabalho a alguém. Ele lista o que o arquivo contém que você talvez não queira compartilhar, e quantos itens de cada tipo encontrou:

- **Anotações** são notas que pessoas deixaram em células. **Remover Tudo** as retira.
- **Propriedades do documento e informações pessoais** são o autor, a pessoa que salvou por último, a empresa e o gerente. **Remover Tudo** as apaga.
- **Planilhas ocultas** são planilhas que não aparecem como guias. **Mostrar Tudo** as exibe, para que você veja o que contêm, e exclua as que não quiser.
- **Linhas e colunas ocultas** são linhas que você ocultou manualmente. Linhas ocultas por um filtro não são contadas. **Mostrar Tudo** as exibe, para que você veja o que contêm.
- **Cabeçalhos e rodapés** são os textos impressos no alto e na base de cada página. **Remover Tudo** os apaga em todas as planilhas.
- **Links externos** são vínculos a valores de outras pastas de trabalho. **Remover Tudo** transforma cada célula que lê um deles no número ou texto que ela mostra agora, e deixa de fora os vínculos ao salvar. Um nome que representava um local em outra pasta de trabalho vira **#REF!**, um erro que indica que o local não existe mais. Isso não pode ser desfeito, então salve uma cópia antes se talvez quiser os vínculos de volta.
- **Dados XML personalizados** são dados que outros programas acrescentaram ao arquivo e que você não vê em uma planilha. **Remover Tudo** os deixa de fora ao salvar.

Salve a pasta de trabalho depois para manter a cópia limpa.

**Remover informações pessoais deste arquivo ao salvar**, abaixo dos cartões, faz isso toda vez que você salva, em qualquer arquivo, sem alterar o documento aberto. É a mesma caixa das **Opções**, e igual em todos os aplicativos do JeffOffice.

## Verificar Compatibilidade para .csv

Escolha **.csv** em **Verificar Compatibilidade** para ver o que um arquivo .csv deixaria de fora da sua pasta de trabalho. Um arquivo .csv guarda apenas os valores de uma planilha. A lista informa as outras planilhas, as fórmulas (o arquivo guarda os resultados), as imagens, gráficos, anotações, tabelas dinâmicas e o restante que sua pasta de trabalho tem, os nomes definidos e as tabelas, e sempre tudo o que não é valor, como formatos de número, fontes, cores, bordas, larguras de coluna, células mescladas, painéis congelados, filtros, formatação condicional, validação de dados, links e configurações de impressão. Também lista as escolhas guardadas nas propriedades do arquivo, como Marcar como Final e Sempre Abrir como Somente Leitura. Não salva nada. Escolha **Salvar como .xlsx** ao salvar para mantê-los todos.

## Verificar Compatibilidade

Clique em **Verificar Compatibilidade** e escolha **.xls**, **.ods** ou **.csv** para ver o que esse tipo de arquivo não conseguiria guardar. A lista mostra as partes da pasta de trabalho que seriam deixadas de fora ou cortadas para caber, como gráficos, anotações ou linhas além do antigo limite, e as escolhas guardadas no próprio arquivo, Marcar como Final e Sempre Abrir como Somente Leitura, quando estiverem ativas. Não salva nada, e sua pasta de trabalho permanece como está.
