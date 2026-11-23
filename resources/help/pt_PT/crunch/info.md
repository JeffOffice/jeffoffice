# Informações: propriedades, palavras-passe e Inspecionar

<!-- keywords: informações, propriedades, título, autor, assunto, palavras-chave, categorias, comentários, empresa, gestor, propriedades personalizadas, marcar como final, final, só de leitura, palavra-passe, encriptar, proteger, inspecionar livro, informações pessoais, folhas ocultas, nomes ocultos, objetos ocultos, notas, ligações externas, XML personalizado, mostrar na pasta, copiar caminho, pasta, caminho, histórico de versões, versões, voltar atrás -->

Clique em **Ficheiro > Informações** para ver os dados do seu livro de cálculo e protegê-lo antes de o partilhar. Prima **Esc** para regressar ao livro.

## Onde está o ficheiro

No topo, a página mostra onde o ficheiro está guardado. Clique em **Mostrar na Pasta** para abrir essa pasta. Clique em **Copiar Caminho** para copiar o caminho, de modo a poder colá-lo num correio eletrónico ou numa conversa.

## Propriedades

As propriedades são dados sobre o livro de cálculo, como o seu título e quem o escreveu. A página tem uma caixa para cada uma: **Título**, **Autor**, **Assunto**, **Palavras-chave**, **Categorias**, **Comentários**, **Empresa** e **Gestor**. Escreva numa caixa e prima **Tab** ou **Enter**. O JeffCrunch mantém a alteração e grava-a no ficheiro da próxima vez que guardar o livro. A página mostra também quando o livro foi criado, guardado e impresso.

Clique em **Propriedades Personalizadas** para criar propriedades próprias, como **Projeto** ou **Cliente**. Clique em **Adicionar**, escreva um nome, escolha o tipo de valor (texto, número inteiro, número, sim ou não, ou data) e escreva o valor. O JeffCrunch guarda-as tal como as escreveu.

## Marcar como Final

Clique em **Marcar como Final** para indicar aos leitores que o livro está concluído. O JeffCrunch guarda o livro e, depois, mantém-no apenas para leitura. Quando alguém o abre, a barra de título indica que é só de leitura. Trata-se de uma forma suave de dizer "por favor, não altere isto". Não é um bloqueio, porque qualquer pessoa pode escolher **Marcado como Final** novamente para o editar.

## Abrir Sempre Só para Leitura

Clique em **Abrir Sempre Só para Leitura** para que o livro seja aberto apenas para leitura de cada vez, evitando alterações acidentais. A escolha é guardada com o ficheiro quando o guardar. Da próxima vez que alguém abrir o ficheiro, a barra de estado indica que está aberto apenas para leitura. Para o editar, abra **Ficheiro > Informações** e clique em **Abre Sempre Só para Leitura**. É um clique apenas, por isso trata-se de um lembrete e não de um bloqueio. Outros programas não leem esta escolha. Um ficheiro de texto, um ficheiro .xls e um ficheiro .ods não a podem guardar, por isso guarde primeiro o livro como ficheiro .xlsx.

## Encriptar com Palavra-passe

Clique em **Encriptar com Palavra-passe** para pedir uma palavra-passe sempre que o ficheiro for aberto. É o mesmo que **Proteger com Palavra-passe**. Veja [Abrir livros protegidos por palavra-passe](open-password). Uma palavra-passe protege um ficheiro **.xlsx** com uma proteção forte. Uma palavra-passe **não pode ser recuperada** se a esquecer. As versões que guardou e a cópia de segurança seguem a palavra-passe.

Para impedir alterações a uma folha ou ao seu esquema, veja [Proteger uma folha ou o livro](protect).

## Proteger a folha ou as folhas

**Proteger Folha Atual** permite escolher o que as pessoas podem alterar na folha em que está, com uma palavra-passe, se quiser. **Proteger Estrutura do Livro** impede que as pessoas adicionem, eliminem, movam, ocultem ou renomeiem folhas. Ambas também estão no separador **Rever**.

## Histórico de Versões

**Histórico de Versões** guarda cópias do seu livro neste computador. Nada é enviado para lado nenhum. Uma cópia chama-se versão. Guarde uma antes de uma alteração grande.

- **Guardar uma Versão** guarda o livro tal como está agora, com um nome à sua escolha.
- **Abrir uma Cópia** abre uma versão numa janela própria, para a poder ver ao lado do livro.
- **Voltar a Esta Versão** repõe uma versão como o seu livro. O livro tal como está agora fica guardado primeiro como versão, para que nada se perca.
- **Eliminar** descarta uma versão.

O livro tem de ser guardado primeiro, porque as versões pertencem a um ficheiro. Voltar atrás funciona com um ficheiro .xlsx.

## Inspecionar Livro

Clique em **Inspecionar Livro** antes de enviar um livro a alguém. Esta opção lista o que o ficheiro contém e que pode não querer partilhar, e quantos itens de cada tipo encontrou:

- **Notas** são notas que as pessoas deixaram em células. **Remover Tudo** elimina-as.
- **Propriedades do documento e informações pessoais** são o autor, a pessoa que o guardou pela última vez, a empresa e o gestor. **Remover Tudo** limpa-os.
- **Folhas ocultas** são folhas que não aparecem como separadores. **Mostrar Tudo** mostra-as, para ver o que contêm, e pode eliminar as que não quer.
- **Linhas e colunas ocultas** são linhas que ocultou manualmente. As linhas ocultas por um filtro não são contadas aqui, e a linha indica quantas são: limpe o filtro para as mostrar. **Mostrar Tudo** mostra as restantes, para ver o que contêm.
- **Nomes ocultos** são nomes definidos como ocultos, pelo que as listas de nomes não os apresentam. **Mostrar Tudo** mostra-os todos de uma vez, o que pode anular, para ver o que representam. Continuam no livro, porque uma fórmula pode lê-los.
- **Objetos ocultos** são gráficos, imagens, formas e caixas de texto definidos como ocultos, pelo que não aparecem nem são impressos. **Remover Tudo** retira-os do ficheiro ao guardar, juntamente com as imagens e os dados de gráfico que só eles usavam. Esta ação não pode ser anulada. Inclui uma forma oculta dentro de um grupo que esteja visível. Se uma linha abaixo da contagem disser que alguns desenhos são demasiado grandes para serem verificados por completo, ou que formas de um tipo mais antigo não podem ser lidas, a contagem não está completa, e a linha indica "Não totalmente verificado" em vez de "Nenhum encontrado".
- **Cabeçalhos e rodapés** são o texto impresso no topo e na base de cada página. **Remover Tudo** limpa-os em todas as folhas.
- **Ligações externas** são ligações a valores de outros livros. **Remover Tudo** transforma cada célula que lê uma delas no número ou texto que mostra agora, e deixa as ligações de fora ao guardar. Um nome que representava um local noutro livro passa a **#REF!**, um erro que indica que o local já não existe. Não é possível anular esta ação, por isso guarde primeiro uma cópia se pretender voltar a ter as ligações.
- **Dados XML personalizados** são dados que outros programas acrescentaram ao ficheiro, e que não são visíveis numa folha. **Remover Tudo** deixa-os de fora ao guardar.

Guarde o livro depois para manter a cópia limpa.

**Remover informações pessoais deste ficheiro ao guardar**, abaixo dos cartões, faz isto sempre que guardar, para qualquer ficheiro, sem alterar o documento que tem aberto. É a mesma caixa que está em **Opções**, e é igual em todas as aplicações do JeffOffice.

## Verificar Compatibilidade para .csv

Escolha **.csv** em **Verificar Compatibilidade** para ver o que um ficheiro .csv deixaria de fora do seu livro. Um ficheiro .csv contém apenas os valores de uma folha. A lista indica as outras folhas, as fórmulas (o ficheiro tem os respetivos resultados), as imagens, os gráficos, as notas, as tabelas dinâmicas e o restante que o livro tem, os nomes definidos e as tabelas, e sempre tudo o que não seja valor, como formatos de número, tipos de letra, cores, limites, larguras de colunas, células unidas, painéis fixos, filtros, formatação condicional, validação de dados, ligações e definições de impressão. Indica também as escolhas guardadas nas propriedades do ficheiro, como Marcar como Final e Abrir Sempre Só para Leitura. Não guarda nada. Escolha **Guardar como .xlsx** ao guardar para manter tudo.

## Verificar Compatibilidade

Clique em **Verificar Compatibilidade** e escolha **.xls**, **.ods** ou **.csv** para ver o que esse tipo de ficheiro não conseguiria conter. A lista indica as partes do livro que ficariam de fora ou seriam cortadas para caber, como gráficos, notas ou linhas além do antigo limite, e as escolhas guardadas no próprio ficheiro, Marcar como Final e Abrir Sempre Só para Leitura, quando estiverem ativas. Não guarda nada, e o seu livro mantém-se como está.
